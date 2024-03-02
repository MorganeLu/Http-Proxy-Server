# Set the base image to Ubuntu
FROM ubuntu:20.04
LABEL maintainer="Kaixin Lu && Qianyi Xue"
RUN mkdir ../var/log/erss
RUN touch ../var/log/erss/proxy.log
WORKDIR /code
COPY . .
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*
RUN ["chmod", "+x", "run.sh"]
ENTRYPOINT ["./run.sh"]

