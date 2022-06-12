if(TOP_LEVEL_PROJECT)
    find_package(Doxygen COMPONENTS dot)
    if(DOXYGEN_FOUND)
        include(FetchContent)
        FetchContent_Declare(doxygen_awesome_css
            GIT_REPOSITORY "https://github.com/jothepro/doxygen-awesome-css.git"
            GIT_TAG 4cd62308d825fe0396d2f66ffbab45d0e247724c # v2.0.3
        )
        FetchContent_MakeAvailable(doxygen_awesome_css)
        set(DOXYGEN_GENERATE_TREEVIEW YES)
        set(DOXYGEN_HTML_EXTRA_STYLESHEET
            "${doxygen_awesome_css_SOURCE_DIR}/doxygen-awesome.css"
            "${doxygen_awesome_css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css"
        )
        set(DOXYGEN_EXCLUDE_SYMBOLS detail)
        doxygen_add_docs(docs include "${OUTPUT_INCLUDEDIR}" docs)
    endif()
endif()
