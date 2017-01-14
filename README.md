# Trivial-File-Transfer-Protocol

FASE 1
Na	primeira fase	 deve implementar um	 cliente	 e servidor	 TFTP	 com	suporte multi-processo.	
Deve	 acrescentar à sua	 implementação	 a	funcionalidade	de listar	ficheiros	 do diretório	remoto, fazendo	 as	 alterações	 que	 considerar	 necessárias	 tanto	 no cliente	 como	 no	 servidor. O	 cliente	 deve	 receber	o	comando	 por argumento	 (“ls nomeDiretório”)	 e	enviá-lo para	o servidor.	 Por	fim,	 o cliente	 irá receber	uma	mensagem de	volta com a	listagem	dos	ficheiros	contidos	na	 diretória do	servidor.	 Caso a	diretoria	não	exista	 deverá devolver	uma	mensagem	 com	essa	indicação ao cliente.	
O	 código	 do	 cliente	 deve	 ser	 alterado	 de	 forma	 que	 este	 solicite ao	 servidor,	 de	 forma	 concorrente, a	transferência	de	todos	os	ficheiros	do	diretório	indicado (e.g.,	 mget nomeDiretório	 – multiple	get).	 O	 cliente	 vai	 então	 lançar	 N processo	 filho	 para solicitar N ficheiros,	 e	 depois	 aguardar que	 todos	 os	 seus	 filhos	 terminem. Por fim, antes	 de	 terminar, este	 deve também imprimir o	tempo	de	execução.	Cada	filho	vai	efetuar	 um pedido	ao	servidor	e	guardar o	ficheiro recebido	 num	 diretório	 corrente.	 Deve	 registar	 os	 tempos	 obtidos,	 na	 mesma	 máquina,	 entre	
máquinas.
O	código	do	servidor	deve	ser	alterado	de	forma	a	que	este	 inicie um	 processo	para	 atender	cada	 pedido.	 O	 processo	 principal	 (que	 recebe	 os	 pedidos)	 deve	 ignorar	 o	 estado	 de	 finalização	 dos	 seus	 filhos	 (ver	 sinal	 SIGCHLD).	 Depois	 de	 atender	 um	 pedido	 o	 processo	 filho	 simplesmente	 termina.

FASE 2
Na	segunda fase	 o	 objectivo é	 tornar	 o	 cliente	 e	 servidor	 TFTP	 em	 aplicações	 multi-tarefa.	 O	 comportamento	 do	 cliente	 e	 do	 servidor	 HTTP	 será	 o	 mesmo	 que	 foi	 implementado	 na fase	 1,	mas	 desta	 vez	 usado	 tarefas	 POSIX.	 Os	 mesmos	 testes	 devem	 ser	 efectuados	 (cf.	 fase	 1).	 O	 servidor	 será	 alterado	 para	 lançar uma	 tarefa	 para	 atender	 cada	 novo	 pedido. Use	 diretivas de	 pré-processamento,	nomeadamente	Macros, para definir	se	a	sua solução recorre	ao	paradigma	 de	 programação multi-processo	 ou	 multi-tarefa (e.g.,	 #define	 OperationMode	 0	 //	 0	 is	 multiprocess	 or	1 is	multi-thread).

FASE 3
O servidor	 deve ser	alterado	de	forma	a	que	coloque	em	execução	 N tarefas	 para	 atender pedidos	 (worker	threads).	 Estas	 tarefas	 vão	 funcionar	 como	 consumidoras	 enquanto	 a	 tarefa	 principal	 (que	 recebe	 os	 pedidos	 vindos	 de	 Internet)	 funcionará como	 produtora.	 O	produtor	vai	colocar	os	pedidos	num	 buffer de	tamanho	 M. Os	consumidores	retiram	 do	 buffer os	pedidos	 para	 os	 atenderem.	 Trata-se	 de	 uma	 implementação	 do	 problema	 produtor	 /	
consumidor	que	 é	revisto nas	aulas.	Os	valores	 N e	 M são	passados	ao	servidor	 por	argumento	através	 da	 linha	 de	 comandos.	 No	 fim,	 deve	 implementar uma	 tarefa	 adicional	 que	 imprime
periodicamente no terminal	o	estado atual do	 buffer.
