idf_component_register(
    SRCS
        "http_server.c"
    INCLUDE_DIRS
        "include"
    PRIV_REQUIRES
        main
    EMBED_FILES "index.html"
    EMBED_FILES "favicon.ico"
)
