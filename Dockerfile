FROM ubuntu:22.04

# Set environment to non-interactive
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy source code
COPY . .

# Clean build directory if it exists
RUN rm -rf project/build

# Create build directory and configure
RUN mkdir -p project/build && \
    cd project/build && \
    cmake -S .. -B . -DTARGET_BUILD=host -DCMAKE_BUILD_TYPE=Release

# Build
RUN cd project/build && \
    cmake --build . -j$(nproc)

# Run tests if available
RUN cd project/build && \
    ctest --output-on-failure || true

# Install to a staging directory for packaging
RUN cd project/build && \
    mkdir -p /staging && \
    cmake --install . --prefix /staging

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    libboost-program-options1.74.0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=0 /staging /usr/local

# Symlink config to /etc for backward compatibility
RUN ln -s /usr/local/etc/atmolyt /etc/atmolyt

ENTRYPOINT ["/usr/local/bin/atmolyt-host"]
