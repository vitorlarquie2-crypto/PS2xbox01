<!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:5e21136c0e6a233ab3a5cf9c807ba7a42ce6ad52d6b27a47fe87c7ac05b24355 -->

# Auditoria dos objetos usados no diagnóstico local

## Problema confirmado

O executável Linux de diagnóstico com a correção de pilha VU0 foi ligado com um conjunto de objetos anterior à correção de CVT.W.S. Uma segunda ligação, com a otimização de máscaras VU, preservou esses objetos antigos. Isso foi um erro no procedimento local de investigação.

Na execução antiga, a rotina original `StageManagerCheckAreaExist` arredondava uma coordenada intermediária de 5,5 para 6. A consulta retornava área ausente e o jogo deixava de chamar o preprocessamento de geometria de Wander. Não houve alteração da posição, dos flags ou das regras do jogo para contornar a condição.

## Verificação

Foi compilado o corpo original da rotina de área com uma fixture sintética: grade 10×10, células 600×600, subgrade 4×4, posição (-300,11,-278) e flags 5 na subcélula esperada. A versão com a macro atual selecionou a célula X5, índice local 10 e retornou área 1 em quatro modos de arredondamento do host. O endereço de retorno guest foi preservado. Compilação e execução: exit 0.

A macro antiga baseada em `nearbyintf` retornou área 0 nos modos nearest e upward; a execução de comparação terminou com exit 1, demonstrando a falha. O teste usa RAM isolada e dados sintéticos; não distribui conteúdo do jogo.

O novo executável foi ligado com todos os objetos privados atualizados, o objeto com a correção da pilha VU0 e a biblioteca atual do runtime. A auditoria de código de máquina encontrou quatro conversões truncadas e nenhuma chamada a `nearbyintf` na rotina de área. SHA256 do executável Linux atualizado:

```text
7a6159189f3af8180bbfa4f2f6a07de3567dc36a19fe1d81e885e67f0e0c8865
```

Os comandos, hashes de cada objeto e identidades dos testes estão registrados no contexto privado da tarefa. A compilação UWP foi realizada separadamente. A falha encontrada na ligação Linux não estabelece uma falha equivalente no pacote UWP.

## Limites atuais

<!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:4d0f7ff8fee4341c269919b5dbd6694d798fc487aa330ef7b6fdb25a2a8a62a8 -->

As observações de ausência de movimento nos executáveis Linux ligados com objetos antigos não validam o comportamento do jogo com CVT.W.S corrigida. Os testes públicos independentes continuam válidos para suas entradas registradas.

Na nova partida, a consulta de área retornou 1, célula X5 e índice local 10; a geometria de Wander passou a executar. O personagem ficou visível. Com entradas normais de teclado, foi observado giro com o analógico, salto e aterrissagem. Na sequência com K mantido, a coordenada Y variou de 10,968 a 12,036 e Z passou de -278,043 a -278,702. Ao manter I, o deslocamento mudou de direção: X passou de -300,001 a -299,503 e Z de -278,790 a -278,302. Não houve escrita direta em RAM guest ou bypass de estado.

Uma captura de 60 segundos em tempo real registrou a sequência local de aterrissagem e mudança de direção. A captura tem 640×448 pixels, sem áudio e taxa de 5 quadros/s; essa taxa não mede o FPS do jogo. A execução local é lenta e ainda apresenta problemas gráficos. Os primeiros controles estão demonstrados no Linux. Instalação, áudio, desempenho e jogabilidade física no Xbox permanecem sem confirmação.

<!-- /coderabbit-ai-generated:v1 -->

<!-- /coderabbit-ai-generated:v1 -->
