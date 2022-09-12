set(HEADERS
    epoll_wrapper/Epoll.h
    epoll_wrapper/Epoll.ipp
    epoll_wrapper/Error.h
    epoll_wrapper/Event.h
    epoll_wrapper/Light.h)

install(FILES ${HEADERS} DESTINATION include/epoll_wrapper)