FROM node:18-alpine
WORKDIR /app

COPY package.json ./
COPY faiss-node.tgz ./

RUN npm install ./faiss-node.tgz --verbose
COPY . .

CMD [ "node", "index.js" ]