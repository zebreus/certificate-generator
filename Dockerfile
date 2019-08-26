FROM archlinux/base
MAINTAINER Lennart E.

#get reflector for fastest mirror
RUN pacman -Sy && pacman -S --noconfirm reflector

#get latex, thrift and boost(for thrift)
RUN reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist && pacman -Sy && pacman -S --noconfirm texlive-core thrift boost

#get build tools
RUN reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist && pacman -Sy && pacman -S --noconfirm base-devel

#remove reflector
RUN pacman -Rns --noconfirm reflector

#build certificate generator
COPY ./ /certgen/
RUN make -C /certgen/ thrift && make -C /certgen/ server

#remove build tools
#RUN pacman -Rns --noconfirm base-devel

ENV PORT 9090
ENV CONFIGURATION_FILE /certgen/data/example_base.json
WORKDIR /certgen/
CMD /certgen/out/server -c $CONFIGURATION_FILE -p $PORT $([[ ! -z "$VERBOSE" ]] && echo -v)

#install build tools
#RUN pacman -S --noconfirm base-devel git sudo
#add build user
#RUN useradd builduser -m && passwd -d builduser && printf 'builduser ALL=(ALL) ALL\n' | tee -a /etc/sudoers
#build nlohmann/json from aur
#RUN sudo -u builduser bash -c 'cd ~ && git clone https://aur.archlinux.org/nlohmann-json-bin.git nlohmann-json-bin && cd nlohmann-json-bin && makepkg -si --noconfirm'
