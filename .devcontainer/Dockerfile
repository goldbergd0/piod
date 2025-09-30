# Use a Raspberry Pi-compatible base image (e.g., Debian for ARM)
FROM debian:bookworm as base

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and C++ tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    g++-aarch64-linux-gnu \
    gcc-aarch64-linux-gnu \
    git \
    pkg-config \
 && rm -rf /var/lib/apt/lists/*

FROM base as builder

WORKDIR /app
COPY . .
ENV CC=gcc
ENV CXX=g++
RUN mkdir build
# CMD ["bash", "-c", "if [ -f CMakePresets.json ]; then cmake --preset=release && cmake --build --preset=release; else cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build; fi"]
CMD ["bash"]