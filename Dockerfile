FROM jamesbrink/opengl

RUN apk add --no-cache build-base xorg-server-dev cmake

ARG SRC_DIR=/usr/src/basic_window
COPY . $SRC_DIR
WORKDIR ./build
RUN  cmake -DCMAKE_BUILD_TYPE=Release $SRC_DIR && cmake --build .
ENTRYPOINT ./basic_window
