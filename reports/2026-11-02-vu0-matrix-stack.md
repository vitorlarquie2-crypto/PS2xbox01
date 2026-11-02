<!-- coderabbit-ai-generated:v1 -->

# Correção da pilha de matrizes VU0 — 2026-11-02

## Falha encontrada

<!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:1b7b9edfb11673dfd8a09a1a5269496abf467b4adde9654d847ff29cdef5b497 -->

O rastreamento de um modelo da cena mostrou matrizes de ossos com os três primeiros vetores zerados. A associação desse modelo a Wander não foi estabelecida. A cadeia de produtores levou às rotinas de salvar/restaurar a matriz atual.

<!-- /coderabbit-ai-generated:v1 -->

A tradução de VSQI trocava o vetor fonte Fs pelo registrador de endereço It. Além disso, VSQI e VLQD acessavam endereços baixos da RAM EE, embora essas instruções operem na memória local VU0. Ao retornar de um ramo do esqueleto, a matriz anterior não era restaurada. Esta é uma correção de semântica de instrução; não muda as regras do jogo nem injeta posições em sua memória.

## Alteração

- Loads VLQI/VLQD usam Is como base e Ft como destino; stores VSQI/VSQD usam It como base e Fs como fonte.
- Transferências locais usam a janela EE11004000 e espelham seus4KiB separadamente da aritmética VI de16bits.
- Preservados destino parcial, atualização antes/depois do acesso e registradores zero.
- Oito instruções em dois arquivos privados foram regeneradas pelo recompilador corrigido. Esses arquivos e os dados do jogo permanecem fora do repositório público.

## Verificação

- Baseline dos quatro novos testes:471/475, quatro falhas.
- Após correção:475/475 com GCC15.2 e475/475 com Clang21.1.8.
- Trecho emitido pelo CodeGenerator compilado e executado com AddressSanitizer/UndefinedBehaviorSanitizer: duas matrizes aninhadas, máscaras, espelhamento local, wrap de VI e registradores zero passaram.
- Runner Linux e DLL UWP privada recompilados. A execução visual está em andamento; esta correção e os testes não comprovam jogabilidade.

## Limites

Esta mudança cobre a memória local VU0 usada pela pilha do jogo. A janela especial de acesso aos registradores VU1 não foi implementada nem validada nesta mudança. Instalação e execução no Xbox Series S ainda exigem teste no console.

Referência primária de comportamento: [PCSX2 VUops.cpp, operações SQI/LQD e GET_VU_MEM](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/VUops.cpp).
