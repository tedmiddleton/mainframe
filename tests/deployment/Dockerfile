FROM ubuntu:22.04

# Install dependencies
RUN apt-get update
RUN apt-get install -y sudo
RUN apt-get install -y git
RUN apt-get install -y cmake
RUN apt-get install -y g++
RUN apt-get install -y neovim
RUN apt-get install -y tree

# Get bob account ready
RUN useradd bob
RUN echo "bob:bob" | chpasswd
RUN adduser bob sudo
USER bob
ENV HOME /home/bob
ENV EDITOR nvim
WORKDIR $HOME
RUN echo 'alias la="ls -la"' >> ~/.bashrc
RUN echo 'set completion-ignore-case on' >> ~/.inputrc
RUN echo 'set keymap vi-insert' >> ~/.inputrc
RUN echo 'set editing-mode vi' >> ~/.inputrc
RUN echo '"df": "\e"' >> ~/.inputrc
RUN mkdir -p ~/.config/nvim
RUN echo 'filetype plugin indent on' >> ~/.config/nvim/init.vim
RUN echo 'filetype plugin indent on' >> ~/.config/nvim/init.vim
RUN echo 'set softtabstop=4' >> ~/.config/nvim/init.vim
RUN echo 'set shiftwidth=4' >> ~/.config/nvim/init.vim
RUN echo 'set expandtab' >> ~/.config/nvim/init.vim
RUN echo 'set number' >> ~/.config/nvim/init.vim
RUN echo 'set hidden' >> ~/.config/nvim/init.vim
RUN echo 'set ignorecase' >> ~/.config/nvim/init.vim
RUN echo 'set smartcase' >> ~/.config/nvim/init.vim
RUN echo 'set autoindent' >> ~/.config/nvim/init.vim
RUN echo 'set incsearch' >> ~/.config/nvim/init.vim
RUN echo 'set ruler' >> ~/.config/nvim/init.vim
RUN echo 'set textwidth=80' >> ~/.config/nvim/init.vim
RUN echo 'set history=1000' >> ~/.config/nvim/init.vim
RUN echo 'set formatoptions+=n' >> ~/.config/nvim/init.vim
RUN echo 'set formatoptions+=j' >> ~/.config/nvim/init.vim
RUN echo 'set formatoptions+=r' >> ~/.config/nvim/init.vim
RUN echo 'set formatoptions+=w' >> ~/.config/nvim/init.vim
RUN echo 'set formatoptions-=l' >> ~/.config/nvim/init.vim
RUN echo 'set formatoptions+=2' >> ~/.config/nvim/init.vim
RUN echo 'set nowrapscan' >> ~/.config/nvim/init.vim
RUN echo 'syntax enable' >> ~/.config/nvim/init.vim
RUN echo 'inoremap <C-l> <ESC><C-w>l' >> ~/.config/nvim/init.vim
RUN echo 'inoremap <C-j> <ESC><C-w>j' >> ~/.config/nvim/init.vim
RUN echo 'inoremap <C-k> <ESC><C-w>k' >> ~/.config/nvim/init.vim
RUN echo 'inoremap <C-h> <ESC><C-w>h' >> ~/.config/nvim/init.vim
RUN echo 'tnoremap <C-w>n <C-w>N' >> ~/.config/nvim/init.vim
RUN echo 'set pastetoggle=<F2>' >> ~/.config/nvim/init.vim
RUN echo 'nnoremap df <esc>' >> ~/.config/nvim/init.vim
RUN echo 'inoremap df <esc>' >> ~/.config/nvim/init.vim
RUN echo 'vnoremap df <esc>' >> ~/.config/nvim/init.vim
RUN echo 'cnoremap df <C-c>' >> ~/.config/nvim/init.vim

# Grab mainframe and build it
RUN git clone https://github.com/tedmiddleton/mainframe.git
WORKDIR mainframe
#COPY mfCMakeLists.txt CMakeLists.txt
RUN mkdir _build
WORKDIR _build
RUN cmake ..
RUN make -j9

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
#RUN cmake ..
#RUN cmake -DUSE_MAINFRAME_BUILD_DIR=ON ..
#RUN make




