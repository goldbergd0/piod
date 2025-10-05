# Use a Raspberry Pi-compatible base image (e.g., Debian for ARM)
# FROM arm64v8/debian:bookworm AS base
FROM --platform=linux/arm64 debian:bookworm-slim AS base


# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and C++ tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    # g++-aarch64-linux-gnu \
    # gcc-aarch64-linux-gnu \
    git \
    pkg-config \
    libasound2-dev \
    libspdlog-dev \
    libfftw3-dev \
    libusb-1.0-0-dev \
 && rm -rf /var/lib/apt/lists/*

# FROM arm64v8/ubuntu:latest AS depends
# RUN apt-get update && apt-get install -y \
#     libasound2-dev \
#  && rm -rf /var/lib/apt/lists/*
 
FROM base AS builder

# COPY --from=depends /usr/include /app/depends/include
# COPY --from=depends /usr/lib /app/depends/lib

WORKDIR /app
COPY . .
ENV CC=gcc
ENV CXX=g++
RUN mkdir -p build
# CMD ["bash", "-c", "if [ -f CMakePresets.json ]; then cmake --preset=release && cmake --build --preset=release; else cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build; fi"]
CMD ["bash"]