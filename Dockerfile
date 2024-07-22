FROM debian:latest AS build

RUN apt-get update && apt-get install -y curl libcurl4-gnutls-dev libpqxx-dev cmake make gcc g++ libssl-dev libsasl2-dev
COPY . /app
WORKDIR /app
RUN make clean; make -j 4

FROM debian:latest

RUN apt-get update && apt-get install -y libcurl4-gnutls-dev libpqxx-dev libssl-dev libsasl2-dev

COPY --from=build /app/build/llmback /app/llmback

WORKDIR /app
CMD ["./llmback"]
