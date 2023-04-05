idf_component_register(SRCS "door_control_task.c" "servo.c" "app_main.c"
                    INCLUDE_DIRS "../include/")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
