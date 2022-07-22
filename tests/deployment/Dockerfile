FROM ubuntu:22.04

# Install dependencies
RUN apt-get update
RUN apt-get install -y sudo
RUN apt-get install -y git
RUN apt-get install -y cmake
RUN apt-get install -y g++
RUN apt-get install -y neovim

# Get bob account ready
RUN useradd bob
USER bob
ENV HOME /home/bob
ENV EDITOR nvim
WORKDIR $HOME
RUN echo 'alias la="ls -la"' >> ~/.bashrc
RUN echo 'set completion-ignore-case on' >> ~/.inputrc
RUN echo 'set editing-mode vi' >> ~/.inputrc
RUN echo 'set keymap vi-insert' >> ~/.inputrc
RUN echo '"df": "\e"' >> ~/.inputrc

# Grab mainframe and build it
RUN git clone https://github.com/tedmiddleton/mainframe.git
WORKDIR mainframe
RUN mkdir _build
WORKDIR _build
RUN cmake ..
RUN make

# Install mainframe as root
USER root
RUN make install
USER bob
WORKDIR $HOME

# Create mftest project and build it
RUN mkdir mftest
WORKDIR mftest
COPY CMakeLists.txt CMakeLists.txt
COPY mftest.cpp mftest.cpp
RUN mkdir _build
WORKDIR _build
RUN cmake ..
RUN make




