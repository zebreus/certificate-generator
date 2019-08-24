FROM archlinux/base
MAINTAINER Lennart E.

#update db and improve mirrorlist
RUN pacman -Sy && pacman -S --noconfirm reflector && reflector --latest 200 --sort rate --save /etc/pacman.d/mirrorlist && pacman -Sy

#get latex, thrift and boost(for thrift)
RUN pacman -S --noconfirm texlive-core thrift boost

#get build tools
RUN pacman -S --noconfirm base-devel

#install certificate generator
COPY ./ /certgen/
RUN make -C /certgen/ thrift && make -C /certgen/ server

EXPOSE 9090
WORKDIR /certgen/
CMD /certgen/out/server

#install build tools
#RUN pacman -S --noconfirm base-devel git sudo
#add build user
#RUN useradd builduser -m && passwd -d builduser && printf 'builduser ALL=(ALL) ALL\n' | tee -a /etc/sudoers
#build nlohmann/json from aur
#RUN sudo -u builduser bash -c 'cd ~ && git clone https://aur.archlinux.org/nlohmann-json-bin.git nlohmann-json-bin && cd nlohmann-json-bin && makepkg -si --noconfirm'
