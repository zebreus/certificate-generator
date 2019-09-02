FROM archlinux/base
MAINTAINER Lennart E.

#get reflector for fastest mirror
RUN pacman -Sy && pacman -S --noconfirm reflector

#get latex, thrift, boost(for thrift), docker and the build tools
RUN reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist && pacman -Sy && pacman -S --noconfirm texlive-core thrift boost base-devel docker

#build certificate generator
COPY ./ /certgen/
RUN make -C /certgen/ thrift && make -C /certgen/ server

#remove build tools and reflector
RUN pacman -Rns --noconfirm reflector

ENV PORT 9090
ENV CONFIGURATION_FILE /certgen/data/example_base_2.json
ENV KEEP_FILES false
ENV DONT_CRASH true
ENV DEBUG false
ENV INFO false
ENV ERROR false
ENV QUIET false
ENV LOG_DIRECTORY /log/
ENV LOG_DEBUG false
ENV LOG_INFO false
ENV LOG_ERROR false
ENV LOG_QUIET false
#ENV DOCKER true
#ENV THREADS true
#ENV MAX_BATCH_COMPILERS 8
#ENV MAX_COMPILER_MEMORY 1000000000
#ENV MAX_COMPILER_CPU_TIME 10
#ENV COMPILER_TIMEOUT 30
#ENV BATCH_TIMEOUT 300
#ENV MAX_COMPILERS 8

WORKDIR /certgen/
ENTRYPOINT /certgen/out/server -c $CONFIGURATION_FILE -p $PORT \
	--dont-crash=$DONT_CRASH --keep-files=$KEEP_FILES \
	--quiet=$QUIET --debug=$DEBUG --info=$INFO --error=$ERROR \
	--log-directory=$LOG_DIRECTORY  --log-quiet=$LOG_QUIET \
	--log-debug=$LOG_DEBUG --log-info=$LOG_INFO --log-error=$LOG_ERROR \
	$( [[ -n "${DOCKER++}" ]] && echo -n --use-docker && [[ -n $DOCKER ]] && echo -n =$DOCKER ) \
	$( [[ -n "${THREADS++}" ]] && echo -n --use-threads && [[ -n $THREADS ]] && echo -n =$THREADS ) \
	$( [[ -n "${MAX_COMPILER_MEMORY++}" ]] && echo -n --max-compiler-memory=$MAX_COMPILER_MEMORY ) \
	$( [[ -n "${MAX_COMPILER_CPU_TIME++}" ]] && echo -n --max-compiler-cpu-time=$MAX_COMPILER_CPU_TIME ) \
	$( [[ -n "${MAX_BATCH_COMPILERS++}" ]] && echo -n --max-batch-compilers=$MAX_BATCH_COMPILERS ) \
	$( [[ -n "${MAX_COMPILERS++}" ]] && echo -n --max-compilers=$MAX_COMPILERS ) \
	$( [[ -n "${COMPILER_TIMEOUT++}" ]] && echo -n --compiler-timeout=$COMPILER_TIMEOUT ) \
	$( [[ -n "${BATCH_TIMEOUT++}" ]] && echo -n --batch-timeout=$BATCH_TIMEOUT )
