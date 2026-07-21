# Codexion — Fundamentos (notas de estudo)

Um mapa do projeto antes de escreveres qualquer código: o que simula, os
conceitos de threading por baixo, como as peças encaixam, e por que ordem
construir tudo. Escrito para quem é novo em pthreads, por isso começa nos
fundamentos a sério e vai subindo por camadas.

---

## Parte 1 — O que é este projeto, na verdade?

### 1.1 Descodificar a metáfora

O Codexion é uma **simulação de partilha de recursos** disfarçada com uma
história de "coders num espaço de cowork". Tira-lhe o fato e é o clássico
problema dos **Filósofos a Jantar** (*Dining Philosophers*) com regras extra.
Tabela de tradução:

| Palavra da história | O que realmente é                                   |
|---------------------|-----------------------------------------------------|
| coder               | uma thread                                           |
| dongle              | um recurso partilhado protegido por um lock (um "garfo") |
| compilar            | a ação crítica que precisa de **dois** recursos      |
| debugar / refatorar | tempo a "pensar" — sem segurar nada                  |
| burnout             | uma thread que esperou demasiado → a simulação tem de parar |
| o monitor           | uma thread vigia que deteta o burnout a tempo        |

A disposição é **circular**: o coder 1 senta-se ao lado do coder N. Entre cada
*par* de vizinhos há exatamente **um dongle**. Logo, há N dongles para N coders,
e para compilar, um coder precisa do dongle à sua **esquerda** e do à sua
**direita** — os dois ao mesmo tempo. Dois vizinhos nunca podem compilar em
simultâneo porque iam disputar o dongle partilhado entre eles. Essa disputa por
um recurso partilhado é o coração inteiro do projeto.

### 1.2 As condições de vitória/derrota

A simulação corre até acontecer **uma** de duas coisas:

- **Derrota:** algum coder faz *burnout* — ou seja, passaram `time_to_burnout`
  ms desde que esse coder começou a compilar pela última vez (ou desde o início
  da simulação), sem voltar a começar a compilar. Imprime `X burned out` e para.
- **Vitória:** todos os coders compilaram pelo menos `number_of_compiles_required`
  vezes. Para de forma limpa.

A vida de um coder é um ciclo: **adquirir dois dongles → compilar → libertar os
dongles → debugar → refatorar → repetir.** O relógio do burnout só é reposto
quando *começam a compilar*. Debugar e refatorar gastam tempo sem repor esse
relógio — por isso um coder que não consegue dongles durante demasiado tempo
morre.

### 1.3 O caso limite deliberadamente traiçoeiro

Se `number_of_coders == 1`: só há **1** dongle, mas compilar precisa de **2**.
Por isso o coder único *nunca* consegue compilar e **tem** de fazer burnout. O
teu programa tem de lidar com isto e terminar de forma limpa (imprimir o
burnout, parar) — e não ficar pendurado para sempre à espera de um segundo
dongle. Guarda este caso na cabeça; ele parte designs ingénuos.

---

## Parte 2 — Fundamentos de threading (os conceitos que tens de dominar)

És novo em threads, por isso esta secção é o rés-do-chão. Tudo o resto assenta
aqui.

### 2.1 O que é uma thread?

Um programa normal corre uma instrução a seguir à outra — uma única linha de
execução. Uma **thread** é uma linha de execução independente que corre *dentro
do mesmo processo*, partilhando a mesma memória. Com pthreads podes ter várias
threads a correr "ao mesmo tempo" (verdadeiramente em paralelo em vários cores,
ou intercaladas pelo escalonador do SO num só core).

- `pthread_create(&tid, NULL, fn, arg)` — arranca uma nova thread que corre
  `fn(arg)`. No Codexion, cada coder é uma chamada a `pthread_create`, mais uma
  para o monitor.
- `pthread_join(tid, NULL)` — espera (bloqueia) até essa thread terminar. O
  `main` usa isto para esperar por todos antes de limpar tudo.

**O perigo central:** como as threads *partilham memória*, duas delas podem
tocar na mesma variável ao mesmo tempo. É daí que vêm os bugs.

### 2.2 A race condition (porque partilhar memória é perigoso)

Imagina `count++`. Parece atómico mas são na verdade três passos: ler count,
somar um, voltar a escrever. Se duas threads fazem `count++` ao mesmo tempo,
ambas podem ler o mesmo valor antigo, ambas somam um, ambas escrevem — e perdes
um incremento. Isso é uma **race condition**: o resultado depende do timing
imprevisível das threads. Agora imagina dois coders vizinhos a verificar ambos
"este dongle está livre?" no mesmo instante — ambos veem "livre", ambos o
agarram. Dois coders a segurar o mesmo dongle = um recurso duplicado = um bug. O
enunciado proíbe isto explicitamente.

### 2.3 O mutex (lock de exclusão mútua)

Um **mutex** é um lock que só uma thread pode segurar de cada vez. O padrão:

```c
pthread_mutex_lock(&m);   // espera até eu ter o lock, depois avança
/* secção crítica: só UMA thread corre isto de cada vez */
pthread_mutex_unlock(&m); // liberta para outra thread poder entrar
```

Tudo *entre* o lock e o unlock é uma **secção crítica** — protegida. Se o estado
do dongle vive atrás de um mutex, a sequência "verificar se está livre / agarrar"
torna-se indivisível: dois coders não conseguem ambos vê-lo livre. Regra de
ouro: um pedaço de dados partilhados e o mutex que o protege andam juntos; só
lês ou escreves esses dados enquanto seguras o respetivo lock.

Tens: `pthread_mutex_init` (configurar), `lock`, `unlock`, `destroy`
(desmontar). Todo o mutex que inicializas tem de ser destruído no fim.

### 2.4 A variável de condição (esperar de forma eficiente)

Um mutex impede duas threads de colidirem, mas não ajuda no *"esperar até algo
se tornar verdadeiro"*. Exemplo: um coder quer um dongle que está agora ocupado.
Má solução: ciclo infinito a verificar — `while not free: verificar outra vez` —
isto faz **busy-wait**, queimando 100% de CPU para nada. Proibido em espírito e
horrível na prática.

Uma **variável de condição** deixa uma thread *dormir* até outra thread
sinalizar "algo mudou, vai reverificar". O padrão canónico — decora esta forma,
vais usá-la constantemente:

```c
pthread_mutex_lock(&m);
while (!condicao_verdadeira)        // SEMPRE um while, nunca um if
    pthread_cond_wait(&cv, &m);     // atomicamente: liberta m, dorme; ao acordar, volta a fazer lock de m
/* a condição é agora verdadeira E eu seguro o lock */
pthread_mutex_unlock(&m);
```

Três coisas para gravar na memória:

1. **O `cond_wait` liberta o mutex enquanto dorme** e volta a adquiri-lo ao
   acordar. É por isso que tens de segurar o lock antes de o chamar. É esta a
   magia que permite que *outras* threads avancem (e te sinalizem eventualmente)
   enquanto esperas.
2. **Reverifica sempre num ciclo `while`, nunca num `if`.** Uma thread pode
   acordar e descobrir que a condição *ainda é falsa* (outra thread agarrou o
   recurso primeiro, ou houve um "spurious wakeup"). Por isso, ao acordar, tens
   de voltar a testar o predicado e talvez voltar a dormir.
3. **Alguém tem de te acordar.** A thread que muda o estado partilhado chama
   `pthread_cond_broadcast(&cv)` para acordar quem espera, para reverificarem.
   ⚠️ Nota: a lista de funções permitidas dá-te `cond_broadcast` mas **NÃO**
   `pthread_cond_signal`. Por isso acordas *todos* os que esperam e deixas cada
   um reverificar o predicado — isso é correto, só tens de ter consciência de
   que o signal está fora dos limites.

### 2.5 cond_timedwait — esperar *com um prazo*

`pthread_cond_timedwait(&cv, &m, &abstime)` é como o `cond_wait` mas também
acorda automaticamente quando o relógio de parede chega a `abstime` (um instante
absoluto, não uma duração). Esta é **a ferramenta mais importante para o
monitor.** O monitor não anda a verificar "já houve algum burnout?" a cada
milissegundo (desperdiçado e impreciso). Em vez disso dorme até *exatamente* o
próximo momento possível de burnout, acordando mais cedo só se o prazo de um
coder mudar. Mais na Parte 4.

### 2.6 O tempo, neste projeto

Só te é permitido `gettimeofday()` para relógios (não `clock_gettime`, nem
`nanosleep`). Vais escrever um helper que devolve "agora, em milissegundos"
(combina `tv_sec` e `tv_usec`). Os teus timestamps de log são *relativos ao
início da simulação*. Para *dormir* uma duração fixa (compilar/debugar/refatorar)
usas `usleep` (microssegundos — por isso um valor em ms é `* 1000`).

---

## Parte 3 — Os problemas difíceis que este projeto te obriga a resolver

Estas são as razões pelas quais o projeto é avaliado pela concorrência, não pelo
número de linhas. Percebe *porque* é que cada um é difícil antes de desenhar.

### 3.1 Deadlock — e as quatro condições de Coffman

**Deadlock** = um conjunto de threads todas presas para sempre, cada uma à
espera de um recurso que outra segura. O gatilho típico: cada coder agarra o seu
dongle da **esquerda** em simultâneo, e depois todos esperam para sempre pelo
dongle da **direita** — que é o dongle da esquerda do vizinho. Ninguém avança;
toda a gente segura um, quer um. Congelado.

O deadlock precisa das **quatro** condições de Coffman ao mesmo tempo. Quebra
qualquer uma → sem deadlock:

1. **Exclusão mútua** — um dongle é segurado por um coder de cada vez. (Não
   podes largar esta; é o objetivo.)
2. **Hold and wait** — um coder segura um dongle enquanto espera pelo segundo.
3. **Sem preempção** — não podes arrancar um dongle da mão de um coder.
4. **Espera circular** — o ciclo "toda a gente espera pelo vizinho da direita".

Soluções clássicas (vais provavelmente usar uma): **quebrar a espera circular**
por ordenação — p. ex. coders pares pegam esquerda-depois-direita, coders ímpares
pegam direita-depois-esquerda, para o ciclo não conseguir fechar; ou **quebrar o
hold-and-wait** concedendo *sempre* ambos os dongles em conjunto através de um
árbitro/fila central (adquire dois ou zero, nunca um-e-espera). O enunciado
empurra-te para um design de árbitro por causa do requisito seguinte.

### 3.2 Arbitragem justa + as políticas de escalonamento

Quando vários coders querem o mesmo dongle, não podes simplesmente dá-lo a quem o
SO calhar acordar primeiro — isso é injusto e não-determinístico. O enunciado
exige uma **política**:

- **FIFO** — concede por *ordem de chegada do pedido*. O primeiro a pedir é o
  primeiro a ser servido.
- **EDF (Earliest Deadline First)** — concede ao coder cujo **prazo de burnout é
  o mais próximo**, onde prazo = `last_compile_start + time_to_burnout`. O coder
  mais perto de morrer tem prioridade. É isto que previne o burnout quando é
  viável.

Ambas as políticas são apenas *"escolher o próximo vencedor de um conjunto de
pedidos em espera por uma ordenação"*. Isso é uma **priority queue** (fila de
prioridade).

### 3.3 A heap (tens de a fazer à mão)

Uma **priority queue** responde a "dá-me o item de maior prioridade" depressa. O
enunciado proíbe qualquer PQ da stdlib — implementas uma **heap binária** à mão.
Uma heap é um array onde os filhos de cada nó ficam nos índices `2i+1` / `2i+2`,
mantida parcialmente ordenada para que o "melhor" elemento esteja sempre no
índice 0. Duas operações: **push** (adicionar, depois subir — *bubble up*) e
**pop** (remover o topo, mover o último para o topo, descer — *bubble down*).
Ambas em O(log n).

A parte elegante que o enunciado sugere: **uma estrutura de heap, dois
comportamentos, escolhidos por uma função comparadora.** FIFO = ordenar por um
número de sequência de chegada; EDF = ordenar por prazo. O mesmo código de
push/pop; só trocas *como dois pedidos se comparam*. O EDF também precisa de um
**critério de desempate** (p. ex. id de coder mais baixo ganha) para que prazos
iguais ainda deem uma resposta determinística — o enunciado exige isto
explicitamente para garantir determinismo.

### 3.4 A race do cooldown

Depois de um dongle ser libertado, fica **indisponível durante `dongle_cooldown`
ms** — está "a arrefecer", ainda não agarrável. Por isso "este dongle está
disponível?" não é só "está livre?" — é "está livre **e** já passou o cooldown?".
Isso significa que um dongle pode ficar disponível puramente porque *passou
tempo*, sem nenhuma thread a libertá-lo ativamente. Algo tem de acordar quem
espera quando o cooldown expira (outra vez espera com prazo), e cada thread
acordada tem de **reverificar o predicado completo** (livre E arrefecido) antes
de o agarrar — a regra do ciclo `while` da 2.4 ganha aqui o seu valor.

### 3.5 Deteção precisa de burnout (a regra dos ≤10 ms)

Um log de burnout tem de aparecer **dentro de 10 ms** do instante real do
burnout. Isto proíbe duas abordagens preguiçosas:

- Verificar a cada intervalo fixo (digamos 50 ms) — chegarias até 50 ms atrasado.
- Deixar cada coder verificar o seu próprio relógio — um coder preso à espera de
  dongles não está a correr para verificar nada.

Daí a **thread monitor** dedicada. Conhece o prazo de cada coder
(`last_compile_start + time_to_burnout`). Calcula o prazo *mais próximo* que aí
vem e faz `cond_timedwait` até exatamente esse instante. Se esse momento chega e
o coder ainda não começou um novo compile → burnout: regista, levanta a flag de
paragem, acorda todos, fim. Se em vez disso um coder *começa a compilar*
(empurrando o seu prazo para mais tarde), sinaliza o monitor para este recalcular
o próximo instante de acordar. Resultado: o monitor dorme quase o tempo todo e
dispara com precisão.

### 3.6 Logging serializado (sem intercalar)

Um `printf` de várias threads ao mesmo tempo pode intercalar a meio de uma linha
("1 is comp2 is debugging..."). O enunciado proíbe isso. Por isso **um único
mutex protege todo o output** — uma thread faz lock, imprime a linha completa
com o timestamp atual, faz unlock. Subtileza: a linha de burnout ainda tem de
cumprir o prazo de 10 ms mesmo que um coder esteja a meio de uma impressão — por
isso um coder que está a imprimir o seu estado não pode segurar um lock que
bloqueie o monitor de imprimir o burnout. Mantém a secção crítica do log
minúscula.

---

## Parte 4 — Como funciona o monitor

Juntando a Parte 2 + 3. O monitor é uma única thread, separada de todos os
coders. O seu ciclo, conceptualmente:

1. Faz lock do estado partilhado.
2. Percorre todos os coders, encontra o prazo **mais cedo** entre os ainda
   vivos. Verifica também a **condição de vitória** (todos compilaram o
   suficiente) e se já disparou algum burnout.
3. Se a simulação deve parar → unlock, sinaliza todos, sai.
4. Caso contrário `pthread_cond_timedwait` até esse prazo mais cedo (ou até um
   coder sinalizar "o meu prazo mudou, recalcula").
5. Ao acordar: reverifica. O coder de prazo mais cedo passou o seu prazo sem
   começar a compilar? → **burnout**: regista, levanta a flag de paragem, faz
   broadcast, sai. Caso contrário volta ao passo 2.

Porquê `timedwait` e não `usleep(10)` num ciclo: precisão *e* CPU. Acorda
exatamente no prazo, e acorda *mais cedo* se um coder começar a compilar (o que
empurra o prazo desse coder para mais tarde, podendo mudar quem é o mais cedo). O
acoplamento entre "coder começa a compilar" e "monitor recalcula" é a parte a
desenhar com cuidado — que campo partilhado muda, e quem faz o broadcast.

---

## Parte 5 — Mapa ficheiro a ficheiro (o teu scaffold)

Para que serve cada ficheiro já criado. O conceito em que assenta entre
parênteses retos.

- **`codexion.h`** — a espinha dorsal. Todas as tuas structs e protótipos. As
  grandes: uma struct de config global (os 8 args parsed + o instante de início
  da sim + o mutex de log + uma flag de paragem), uma struct por coder (id,
  contagem de compiles, timestamp do último início de compile, índices dos seus
  dongles esquerdo/direito, a sua thread), uma struct de dongle (estado:
  livre/ocupado, o instante em que ficou livre para o cooldown, o seu mutex), e a
  struct da heap. *Sem variáveis globais a sério* — passas ponteiros. Este
  ficheiro define o vocabulário que tudo o resto fala. **Desenha-o primeiro.**

- **`parse.c`** — lê os 8 args, valida-os. Rejeita negativos, não-inteiros, e
  qualquer scheduler que não seja exatamente `fifo`/`edf`. O `atoi` é permitido
  mas não deteta erros — vais ter de validar os dígitos à mão. [lógica pura, sem
  threads]

- **`init.c`** — aloca e inicializa tudo: o array de coders, o array de dongles,
  todos os mutexes e variáveis de condição, a heap, o timestamp de início.
  Emparelha cada alloc/init com um teardown planeado. [malloc, mutex_init,
  cond_init]

- **`utils/time.c`** — o helper "agora em ms" (via `gettimeofday`) e
  provavelmente um pequeno wrapper "dormir N ms" sobre o `usleep`. [gettimeofday,
  usleep]

- **`utils/safe.c`** — provavelmente wrappers seguros (malloc com verificação,
  helpers de lock/unlock de mutex) para manter as funções de ≤25 linhas da Norma
  limpas. [malloc]

- **`log.c`** — o logger serializado: lock do mutex de log, imprime
  `<ts> X <estado>`, unlock. Uma função por onde o programa inteiro passa.
  [mutex, printf, time]

- **`heap.c`** — a heap binária feita à mão: init, push (bubble up), pop (bubble
  down), free. Só a estrutura; não sabe nada sobre FIFO vs EDF. [arrays]

- **`heap_cmp.c`** — os comparadores: "o pedido A tem maior prioridade do que o
  B?" Um para FIFO (ordem de chegada), um para EDF (prazo mais cedo, desempate
  por id). A heap chama o que o scheduler escolhido selecionou. [a lógica da
  política]

- **`scheduler.c`** — o árbitro: os coders *pedem* dongles aqui (push para a
  heap), e isto decide *quem realmente os recebe* segundo a ordenação da heap,
  com o cooldown respeitado. É aqui que vivem a prevenção de deadlock e a justiça.
  [mutex, cond, heap]

- **`dongle.c`** — a mecânica de pegar/libertar dongles e a contabilidade do
  cooldown (marcar o instante de libertação; "disponível" significa livre E
  arrefecido). [mutex, cond, time]

- **`coder.c`** — a função da thread coder: o ciclo de vida (pedir dois dongles →
  log "has taken a dongle" ×2 → log "is compiling", registar last_compile_start,
  usleep do tempo de compile → libertar dongles → debugar → refatorar → repetir),
  verificando a flag de paragem a cada volta. [o corpo da thread — liga tudo]

- **`monitor.c`** — a Parte 4 acima. O vigia. [cond_timedwait]

- **`main.c`** — o orquestrador: parse → init → arrancar threads dos coders + o
  monitor → join de todos → cleanup. [pthread_create, pthread_join]

- **`cleanup.c`** — destruir cada mutex/cond, libertar cada alocação, em cada
  caminho de saída. A garantia de "sem leaks" vive aqui. [free, destroy]

---

## Parte 6 — Ordem sugerida de construção (não comeces pelo monitor)

O monitor é o *último*, porque vigia estado que tem de existir primeiro.
Constrói de baixo para cima, testando cada camada antes de empilhar a seguinte:

1. **structs do `codexion.h`** — decide o teu modelo de dados. Tudo depende dele.
2. **`parse.c`** — deixa a validação dos args sólida como rocha; é lógica pura,
   fácil de testar isolada, e dá confiança.
3. **`utils/time.c` + `log.c`** — precisas de "agora em ms" e de impressão
   serializada antes de algo visível acontecer. Testa: imprime umas linhas com
   timestamp.
4. **`heap.c` + `heap_cmp.c`** — constrói e testa a heap *sozinha* (faz push de
   uns números, faz pop por ordem) antes de a ligar a threads. Debugar uma heap e
   uma race ao mesmo tempo é uma miséria.
5. **`init.c` / `cleanup.c`** — aloca e liberta tudo de forma limpa. Corre com um
   verificador de leaks com zero threads a trabalhar ainda.
6. **`dongle.c` + `scheduler.c`** — a camada de recursos: pedir, conceder,
   cooldown, justiça. É o núcleo da concorrência.
7. **`coder.c`** — o ciclo de vida. Agora consegues ver logs reais de
   "compiling/debugging". Testa com N≥2 e *sem* pressão de burnout primeiro
   (`time_to_burnout` generoso).
8. **`monitor.c`** — adiciona o burnout preciso + a lógica de paragem por último.
   Agora testa os casos brutais: N=1 (tem de fazer burnout, não pendurar),
   `time_to_burnout` apertado, starvation no EDF.
9. **README.md** — as secções obrigatórias (Description, Instructions, Resources
   incl. uso de IA, Blocking cases handled, Thread synchronization mechanisms).

### Pontos de verificação para te manteres honesto
- Norma: ≤25 linhas/função, ≤5 funções/ficheiro, sem `for`, sem globais.
- Só a lista de funções permitidas — denuncia qualquer `cond_signal`,
  `nanosleep`, `clock_gettime`, `mutex_trylock` perdido.
- Leaks: cada `malloc` libertado e cada mutex/cond destruído em *cada* saída.
- N=1 termina via burnout.
- Logs nunca intercalam; o burnout imprime dentro de 10 ms.

---

## Auto-teste: consegues responder a isto em voz alta?

Se algum parecer pouco firme, é aí que deves escavar antes de codificar.

1. Porque é que um coder precisa de **dois** dongles, e o que impede dois
   vizinhos de compilar ao mesmo tempo?
2. Qual é a diferença entre um coder que está a *debugar* e um que está à
   *espera de um dongle* — e qual deles corre risco de burnout?
3. Porque é que o monitor não pode simplesmente fazer `usleep(10)` num ciclo
   para apanhar burnouts?
4. Qual das quatro condições de Coffman é que o *teu* design vai quebrar, e como?
5. O que é que o comparador do EDF compara exatamente, e porque precisa de um
   critério de desempate?
