<!-- coderabbit-ai-generated:v1 -->

# Iteração das máscaras inteiras VU — 2026-11-02

## Alteração e contrato

Uma amostragem de execução encontrou tempo no cálculo das dependências VU. As máscaras de leitura/escrita contêm apenas os registradores usados pelo par de instruções, mas o runtime percorria VI1..15 a cada chamada. Agora percorre os bits presentes, em ordem crescente, usando C++20 `std::countr_zero`.

VI0 continua excluído. O cálculo dos ciclos de disponibilidade, o writeback e o bypass da primeira escrita conservam a semântica anterior. Não há alteração na precisão FMAC, nas regras do jogo ou nos dados convidados. A mudança afeta VU0/VU1 no runtime; não exige regenerar as funções privadas do jogo.

## Validação

- Comparação exaustiva das 65.536 máscaras: mesma sequência de registradores, exclusão de VI0 e primeira escrita.
- Suíte completa existente: 475/475 com GCC15.2 e 475/475 com Clang21.1.8. Inclui testes de dependências, registradores zero, latências e bypass.
- Microbenchmark com interpretador VU real, memória de código rastreada e laços de instruções IADD/branch e ADD/MUL com dependências. Orçamento200.000ciclos,12 repetições por modo. Cinco execuções alternadas de cada variante, Clang21.1.8 `-O3 -flto=thin -msse4.1`.

| Modo | Mediana anterior | Mediana nova | Razão anterior/nova |
| --- | ---: | ---: | ---: |
| IADD/branch | 0,310016s | 0,229557s | 1,35× |
| ADD/MUL com dependências e IADD/branch | 0,262317s | 0,220105s | 1,19× |

O hash de todo o estado VU final foi idêntico entre variantes em cada modo; ciclos200.000, PC0 e VI15 também iguais. As medições são de código sintético no sandbox. Não medem FPS do jogo nem desempenho no Xbox.

## Execução do jogo

A correção anterior da pilha VU0 permitiu observar matrizes válidas e uma cena com o altar, colunas e Mono deitada. O controle de Wander ainda não foi comprovado. A execução visual continua separada destes testes; pacote assinado e cena visível não constituem validação de jogabilidade ou do console físico.
