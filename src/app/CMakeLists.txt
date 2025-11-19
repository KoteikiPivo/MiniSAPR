qt_add_executable(mini_sapr
                    sapr.cpp sapr.h sapr.ui
                    schemawidget.cpp schemawidget.h
                    filehandler.cpp filehandler.h
                    rodsystemcalculator.cpp rodsystemcalculator.h
                    main.cpp)

target_link_libraries(mini_sapr PRIVATE
                    Qt6::Core
                    Qt6::Gui
                    Qt6::Widgets)

set_target_properties(mini_sapr PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

