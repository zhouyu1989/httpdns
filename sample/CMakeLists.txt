
#aux_source_directory(. DIR_SRCS)

add_executable(httpdns_sample httpdns_sample.c)

target_link_libraries(httpdns_sample httpdns)
target_link_libraries(httpdns_sample pthread curl)

