FILE(REMOVE_RECURSE
  "libGECommon.pdb"
  "libGECommon.a"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/GECommon.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
