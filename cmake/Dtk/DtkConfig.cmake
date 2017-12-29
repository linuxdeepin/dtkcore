foreach(module ${Dtk_FIND_COMPONENTS})
    find_package(Dtk${module})
endforeach()
