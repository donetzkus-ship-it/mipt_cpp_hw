#!/usr/bin/env bash
set -e

mkdir -p /workspace/_deps && cd /workspace/_deps

url="https://sourceforge.net/projects/boost/files/boost/1.85.0/boost_1_85_0.tar.gz/download"
wget -O boost_1_85_0.tar.gz $url
tar xvf boost_1_85_0.tar.gz
cd boost_1_85_0/
apt update
apt install autotools-dev build-essential libboost-all-dev libbz2-dev
./bootstrap.sh
options="toolset=gcc variant=release link=static runtime-link=static threading=multi"
./b2 $options -j2 install
./bootstrap.sh --with-python=python3
./b2 --with-python $options -j2 install
cd ..

apt update
apt install -y \
  cmake \
  git \
  pkg-config \
  python3 \
  python3-dev \
  libgoogle-glog-dev \
  libgtest-dev \
  libbenchmark-dev \
  libicu-dev \
  nlohmann-json3-dev \
  libpqxx-dev \
  libpq-dev \
  libtbb-dev \
  zlib1g-dev

git clone --depth 1 https://github.com/galkahana/PDF-Writer
cmake -S PDF-Writer -B PDF-Writer/output -DCMAKE_BUILD_TYPE=Release
cmake --build PDF-Writer/output -j2
cmake --install PDF-Writer/output

git clone --depth 1 --recurse-submodules https://github.com/lecrapouille/zipper
cmake -S zipper -B zipper/output \
  -DCMAKE_BUILD_TYPE=Release \
  -DZIPPER_SHARED_LIB=ON \
  -DZIPPER_BUILD_DEMOS=OFF \
  -DZIPPER_BUILD_TESTS=OFF
cmake --build zipper/output -j2
cmake --install zipper/output