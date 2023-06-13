ARG BASE_IMAGE=node:18-alpine
FROM $BASE_IMAGE
ARG TARGETPLATFORM
ARG BUILDPLATFORM
RUN echo "Running on $BUILDPLATFORM, building for $TARGETPLATFORM";

WORKDIR /app
COPY package.json ./
COPY faiss-node.tgz ./

RUN set -eux; \
  if [ $TARGETPLATFORM == "linux/arm/v7" ]; then \
    apk add cmake build-base openblas-dev; \
  fi

RUN npm install ./faiss-node.tgz --verbose
COPY . .

CMD [ "node", "index.js" ]