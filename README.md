<h1 align="center"> :file_cabinet: Programação de Sockets </h1>
Trabalho da disciplina SSC0142 Redes de Computadores

## :memo: Descrição
Esse trabalho foi feito como parte avaliativa da matéria Redes de Computadores (SSC0142), cujo objetivo era desenvolver três módulos de um chat de conversas utilizando o protocolo TCP/IP. Ele é uma adaptação das especificações dadas pelo RFC 1459, que define o IRC, ou seja, uma implementação cliente/servidor, em que diversos clientes se comunicam entre si, por intermédio de um servidor.

## :wrench: Tecnologias utilizadas
* Linguagem <b>C++</b>
* Sistema Operacional <b>Ubuntu 20.04 </b>
* Visual Studio Code <b>(VSCode)</b>

## Divisão do repositório:
* <b>Módulo 1:</b> Arquitetura cliente-cliente;
* <b>Módulo 2:</b> Arquitetura cliente-servidor simples;
* <b>Módulo 3:</b> Arquitetura cliente-servidor com implementação de canais;

## :rocket: Rodando o código
Para compilar, basta executar o comando:
```
make
```

Para rodar o repositório, é necessário abrir o terminal na pasta do módulo desejado, escolher a aplicação e rodar o comando:

* Servidor:
```
make server
```

* Cliente:
```
make client
```

* <b>Apenas no Módulo 1:</b>
Como este modulo trata-se de uma arquiterua cliente-cliente, ele só tem uma aplicação implementada
```
make run
```

## :handshake: Membros da equipe
* <a href=https://www.linkedin.com/in/eduardo-fares-a271561a0/> Eduardo Morelli Fares </a> - 12686036
* <a href=https://www.linkedin.com/in/gabrielbatistella/> Gabriel Takeshi Miyake Batistella </a> - 11232198
* <a href=https://www.linkedin.com/in/guilherme-villela/> Guilherme Barros Villela </a> - 12549936
* <a href=https://www.linkedin.com/in/vitor-rinaldini/> Vitor Fernando Rinaldini </a> - 11232305
