FROM ubuntu:latest

ARG MANTIS_VERSION=0.2.8

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    unzip \
    wget \
    ca-certificates \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /mantis

RUN wget -O /tmp/mantisapp.zip https://github.com/allankoechke/mantis/releases/download/v${MANTIS_VERSION}/mantisapp_v${MANTIS_VERSION}_linux.zip \
  && unzip /tmp/mantisapp.zip -d /mantis \
  && chmod +x /mantis/mantisapp \
  && rm /tmp/mantisapp.zip

EXPOSE 80

CMD ["/mantis/mantisapp", "--dev", "serve", "-p", "80"]
