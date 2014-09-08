/*
 * GECom Game State Machine (stack-based)
 * 
 */

// TODO make shit derive from Uncopyable where appropriate

#ifndef GECOM_STATE_HPP
#define GECOM_STATE_HPP

#include <utility>
#include <memory>
#include <functional>
#include <vector>
#include <type_traits>
#include <string>
#include <iostream>
#include <exception>
#include <stdexcept>

#include "GECom.hpp"
#include "Log.hpp"

namespace gecom {

	// forward declaration
	class StateManager;

	// base class for state actions
	class Action {
	public:
		// perform this action
		virtual std::unique_ptr<Action> execute(StateManager &) = 0;
	
		// will this action do anything?
		virtual inline operator bool() {
			return true;
		}
	
		virtual inline ~Action() { }
	};

	// action that does nothing
	class NullAction : public Action {
	public:
		virtual inline std::unique_ptr<Action> execute(StateManager &) {
			return make_unique<NullAction>();
		}
	
		// null action never does anything
		virtual inline operator bool() {
			return false;
		}
	};

	// action that calls into a new state
	template <typename ChildStateT>
	class CallAction : public Action {
	public:
		using return_t = typename ChildStateT::return_t;
		using callback_t = std::function<std::unique_ptr<Action> (const return_t &)>;
		using rethrow_t = std::function<void()>;
		using exhandler_t = std::function<std::unique_ptr<Action> (const rethrow_t &)>;
	
	private:
		std::unique_ptr<ChildStateT> m_callee;
		callback_t m_callback;
		exhandler_t m_exhandler;
	
	public:
		template <typename... ArgTR>
		inline CallAction(ArgTR && ...args) : m_callee(make_unique<ChildStateT>(std::forward<ArgTR>(args)...)) { }
	
		// set the return callback and remove exception handler
		inline void callback(const callback_t &cb) {
			m_callback = cb;
			m_exhandler = exhandler_t();
		}
	
		// set the return callback and exception handler
		inline void callback(const callback_t &cb, const exhandler_t &eh) {
			m_callback = cb;
			m_exhandler = eh;
		}
	
		virtual inline std::unique_ptr<Action> execute(StateManager &sm) override;
	};

	// action that returns from the current state
	template <typename ChildStateT>
	class ReturnAction : public Action {
	public:
		using return_t = typename ChildStateT::return_t;
		using callback_t = typename CallAction<ChildStateT>::callback_t;
	
	private:
		callback_t m_callback;
		return_t m_retval;
	
	public:
		inline ReturnAction(const callback_t &f, const return_t &v) : m_callback(f), m_retval(v) { }
	
		virtual inline std::unique_ptr<Action> execute(StateManager &sm) override;
	};


	// base class of all states
	// so we can define the vector that holds them and call methods on them
	class StateBase {
	public:
		using action_ptr = std::unique_ptr<Action>;
	
		virtual inline void onInit() { }
		virtual inline void onEnter() { }
		virtual inline void onExit() { }
	
		inline void enter() {
			// like this for consistency with init(), and there might be shit
			// that always needs to happen here eventually
			onEnter();
		}
	
		inline void exit() {
			// as for enter()
			onExit();
		}
	
		virtual inline action_ptr updateForeground() {
			return make_unique<NullAction>();
		}
	
		virtual inline void updateBackground() { }
		
		virtual inline void drawForeground() { }

		virtual inline void drawBackground() { }
		
		// will this state completely obscure anything drawn underneath it?
		virtual inline bool opaque() {
			return true;
		}
	
		virtual action_ptr handleException(const std::exception_ptr &pex) {
			std::rethrow_exception(pex);
			return make_unique<NullAction>();
		}
	
		virtual inline ~StateBase() { }
	};

	// user base class for states
	template <typename ReturnT = int>
	class State : public StateBase {
		static_assert(!std::is_same<ReturnT, void>::value, "ReturnT cannot be void");
	public:
		using return_t = ReturnT;
		using callback_t = typename CallAction<State>::callback_t;
		using rethrow_t = typename CallAction<State>::rethrow_t;
		using exhandler_t = typename CallAction<State>::exhandler_t;
	
	private:
		callback_t m_callback;
		exhandler_t m_exhandler;
	
	public:
	
		// this is called when the state is called
		// defined like this because it is only called where the type is fully known
		// init() MUST NOT FAIL TO INSTALL THE CALLBACKS; it can exception after that though
		inline void init(const callback_t &cb, const exhandler_t &eh) {
			m_callback = cb;
			m_exhandler = eh;
			onInit();
		}
	
		// exception handler mechanism, should not be overridden
		virtual inline action_ptr handleException(const std::exception_ptr &pex) override final {
			bool rethrow_called = false;
			rethrow_t rethrow = [&] {
				rethrow_called = true;
				std::rethrow_exception(pex);
			};
			action_ptr action = make_unique<NullAction>();
			if (m_exhandler) {
				// this is allowed to throw, but should use throw_with_nested if it throws a different exception
				action = m_exhandler(rethrow);
			}
			if (!rethrow_called) {
				// not rethrowing the exception doesnt count as handling it, so rethrow for next handler
				std::rethrow_exception(pex);
			}
			return action;
		}
	
		// factory function for null actions
		static inline std::unique_ptr<NullAction> nullAction() {
			return make_unique<NullAction>();
		}
	
		// factory function template for state-call actions
		template <typename ChildStateT, typename... ArgTR>
		static inline std::unique_ptr<CallAction<ChildStateT>> callAction(ArgTR && ...args) {
			return make_unique<CallAction<ChildStateT>>(std::forward<ArgTR>(args)...);
		}
	
		// helper function to attach a callback to a state-call action temporary (and remove exception handler)
		// this is needed because you can't chain methods with unique_ptr
		template <typename ChildStateT>
		static inline std::unique_ptr<CallAction<ChildStateT>> && callback(
			std::unique_ptr<CallAction<ChildStateT>> &&action,
			const typename CallAction<ChildStateT>::callback_t &cb
		) {
			action->callback(cb);
			return std::move(action);
		}
	
		// helper function to attach a callback and exception handler to a state-call action temporary
		// this is needed because you can't chain methods with unique_ptr
		template <typename ChildStateT>
		static inline std::unique_ptr<CallAction<ChildStateT>> && callback(
			std::unique_ptr<CallAction<ChildStateT>> &&action,
			const typename CallAction<ChildStateT>::callback_t &cb,
			const typename CallAction<ChildStateT>::exhandler_t &eh
		) {
			action->callback(cb, eh);
			return std::move(action);
		}
	
		// factory function for state-return actions
		inline std::unique_ptr<ReturnAction<State>> returnAction(const return_t &retval) {
			// MSVC what are you doing? why do i need to qualify this with gecom?
			// why are you suggesting std::make_unique is visible?
			return gecom::make_unique<ReturnAction<State>>(m_callback, retval);
		}
	
		virtual ~State() { }
	};

	// state machine implementation
	class StateManager {
		// these actions need access to internals
		template <typename> friend class CallAction;
		template <typename> friend class ReturnAction;
	
	private:
		// call stack
		// be careful not to try copying the unique_ptrs (use references in for-each)
		std::vector<std::unique_ptr<StateBase>> m_states;
	
		class UpdateAction : public Action {
		public:
			virtual inline std::unique_ptr<Action> execute(StateManager &sm) override {
				if (sm.m_states.empty()) return make_unique<NullAction>();
				
				auto it = sm.m_states.begin();
			
				try {
					// update background states
					for (; it < sm.m_states.end() - 1; ++it) {
						(*it)->updateBackground();
					}
				
				} catch (...) {
					// if background update fails, kill all states on top of the failed one
					// it points to the state that failed, and where exception handling begins
				
					// move to last state to kill
					it++;
				
					// pop dead states
					while (it < sm.m_states.end()) {
						sm.m_states.pop_back();
					}
				
					// now let the exception be handled
					throw;
				}
			
				// update foreground state
				return sm.m_states.back()->updateForeground();
			}
		};
	
		class DrawAction : public Action {
		public:
			virtual inline std::unique_ptr<Action> execute(StateManager &sm) override {
				if (sm.m_states.empty()) return make_unique<NullAction>();
				
				auto it = sm.m_states.end();
				
				try {
					// find topmost opaque state
					for (; it --> sm.m_states.begin(); ) {
						if ((*it)->opaque()) break;
					}
					
					// draw from topmost opaque state upwards, but only background states
					for (; it < sm.m_states.end() - 1; ++it) {
						(*it)->drawBackground();
					}
					
				} catch (...) {
					// if opaque() or draw() fails, kill all states on top of the failed one
					// it points to the state that failed, and where exception handling begins
					
					// move to last state to kill
					it++;
					
					// pop dead states
					while (it < sm.m_states.end()) {
						sm.m_states.pop_back();
					}
					
					// now let the exception be handled
					throw;
					
				}

				// draw foreground state
				sm.m_states.back()->drawForeground();
				
				return make_unique<NullAction>();
			}
		};
	
		void performAction(std::unique_ptr<Action> &&a) {
		
			std::unique_ptr<Action> action = std::move(a);
		
			while (*action) {
				try {
					action = action->execute(*this);
				
				} catch (std::exception &) {
					std::exception_ptr pex = std::current_exception();
				
					// try to handle the exception
					// the failed state is assumed to be the back state
					while (!m_states.empty() && pex) {
						try {
							action = m_states.back()->handleException(pex);
							pex = nullptr;
						} catch (std::exception &) {
							// unable to handle exception, propagate current exception to parent
							pex = std::current_exception();
						}
						// even if we handled the exception, we need to pop because the back state's
						// exception handler is a callback to its parent, so the back state is still dead
						m_states.pop_back();
					}
				
					if (pex) {
						// exception was not handled (got to top of stack), rethrow it
						std::rethrow_exception(pex);
					}
				}
			}
		
		}
	
	public:
		// are we finished?
		inline bool done() {
			return m_states.empty();
		}
	
		// reset and construct first state, args are passed to state ctor
		template <typename FirstStateT, typename... ArgTR>
		void init(ArgTR && ...args) {
			static_assert(std::is_base_of<StateBase, FirstStateT>::value, "FirstStateT must be a StateBase subclass");
			m_states.clear();
			auto action = make_unique<CallAction<FirstStateT>>(std::forward<ArgTR>(args)...);
			action->callback(
				[](const typename FirstStateT::return_t &retval) {
					// this gets called when the first state returns
					log("StateManager") << "exiting nicely: " << retval;
					return make_unique<NullAction>();
				},
				[](const typename FirstStateT::rethrow_t &rethrow) {
					try {
						// this gets called if we exception all the way up through the first state
						rethrow();
						log("StateManager").error() << "exception handler called for non-exception. wat?";
					} catch (std::exception &e) {
						log("StateManager").error() << "exception: " << e.what();
					}
					return make_unique<NullAction>();
				}
			);
			performAction(std::move(action));
		}
	
		// run one update iteration
		void update() {
			if (done()) return;
			performAction(make_unique<UpdateAction>());
		}
	
		// run one draw iteration
		void draw() {
			if (done()) return;
			performAction(make_unique<DrawAction>());
		}

	};

	template <typename ChildStateT>
	inline std::unique_ptr<Action> CallAction<ChildStateT>::execute(StateManager &sm) {
		// defined out-of-class due to dependence on StateManager
		// exit(), init() and enter() are allowed to exception out
		if (!sm.m_states.empty()) {
			sm.m_states.back()->exit();
		}
		// TODO this should probs be required to be noexcept
		sm.m_states.push_back(std::move(m_callee));
		// init() MUST NOT FAIL TO INSTALL THE CALLBACKS; it can exception after that though
		// for exception safety, new state must be on the stack before init()
		static_cast<ChildStateT *>(sm.m_states.back().get())->init(m_callback, m_exhandler);
		sm.m_states.back()->enter();
		return make_unique<NullAction>();
	}

	template <typename ChildStateT>
	inline std::unique_ptr<Action> ReturnAction<ChildStateT>::execute(StateManager &sm) {
		// defined out-of-class due to dependence on StateManager
		// exit(), m_callback() and enter() are allowed to exception out
		std::unique_ptr<Action> action = make_unique<NullAction>();
		sm.m_states.back()->exit();
		// this is assumed to be noexcept
		sm.m_states.pop_back();
		if (m_callback) {
			action = m_callback(m_retval);
		}
		if (!sm.m_states.empty()) {
			sm.m_states.back()->enter();
		}
		return action;
	}

}

#endif // GECOM_STATE_HPP
