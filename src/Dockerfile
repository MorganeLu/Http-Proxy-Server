# Set the base image to Ubuntu
# FROM ubuntu:20.04
# LABEL maintainer="Kaixin Lu && Qianyi Xue"
# RUN apt-get update
# RUN apt-get install -y g++ make
# RUN mkdir ../var/log/erss
# RUN touch ../var/log/erss/proxy.log
# RUN mkdir /code
# ADD . /code
# WORKDIR /code
# RUN ["chmod", "+x", "run.sh"]
# ENTRYPOINT ["./run.sh"]


FROM ubuntu:20.04
LABEL maintainer="Kaixin Lu && Qianyi Xue"
WORKDIR /app
COPY . .
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*
RUN ["chmod", "+x", "run.sh"]
ENTRYPOINT ["./run.sh"]


# # 设置容器启动命令
# CMD ["./your_app_executable"]
