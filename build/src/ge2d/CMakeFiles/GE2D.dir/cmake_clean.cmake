FILE(REMOVE_RECURSE
  "libGE2D.pdb"
  "libGE2D.a"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/GE2D.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
