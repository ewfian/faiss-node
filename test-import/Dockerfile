ARG BASE_IMAGE=node:18-alpine
FROM $BASE_IMAGE
WORKDIR /app

COPY package.json ./
COPY faiss-node.tgz ./

RUN npm install ./faiss-node.tgz --verbose
COPY . .

CMD [ "node", "index.js" ]