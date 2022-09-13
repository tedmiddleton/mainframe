FROM ubuntu:22.04

# Install dependencies
RUN apt-get update
RUN apt-get install -y git
RUN apt-get install -y g++
RUN apt-get install -y cmake
RUN apt-get install -y neovim
RUN apt-get install -y tree
RUN apt-get install -y doxygen
ENV DEBIAN_FRONTEND noninteractive
RUN ln -fs /usr/share/zoneinfo/America/Vancouver /etc/localtime
RUN apt-get install -y tzdata
RUN apt-get install -y python3-sphinx
RUN apt-get install -y python3-sphinx-rtd-theme
RUN apt-get install -y python3-breathe

RUN mkdir /workspace

# Get bob account ready
RUN useradd bob
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

