<!-- coderabbit-ai-generated:v1 -->

# SQRT.S do Emotion Engine e entrada do analógico

## Defeito reproduzido

A tradução de `SQRT.S` usava `fs` como radicando. No Emotion Engine, essa instrução lê `ft` e escreve `fd`. A implementação de referência do [PCSX2](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/FPU.cpp) também lê `ft` em `SQRT_S`.

Na rotina de normalização do analógico, a instrução `0x46020004` deve calcular `f0 = sqrt(f2)`. A entrega gerada calculava `sqrt(f0)`. Em execução real, a entrada do controle chegou como LX=128, LY=1, mas a rotina devolveu direção zero. A posição do personagem permaneceu (-300, 11, -278).

## Correção

O tradutor agora usa `ft`. A API dos helpers e suas regras numéricas permanecem iguais. A mudança restaura a semântica da instrução do EE.

O código privado fornecido continha 130 instruções SQRT.S; 68 usavam um operando diferente do exigido pela instrução original. Esses 68 pontos, em 52 arquivos privados, foram corrigidos a partir dos campos dos opcodes. O código do jogo e os arquivos do disco permanecem fora do Git.

## Verificação

- Duas assertivas cobrem registradores distintos e o opcode real. Antes da correção: 464/465 testes, saída 1. Depois: 465/465 com GCC e Clang, saída 0.
- A rotina privada foi executada isoladamente com os mesmos helpers do runtime. Centro, cima, direita e diagonal passaram. A versão antiga falhou ao devolver zero para cima; a corrigida devolveu aproximadamente (0.003901, -0.986933). Na diagonal, devolveu (-0.707107, -0.707107).
- O runner completo foi reconstruído. SHA-256: `3243fd4450a31c789ae8b360382691e3154d700aef7853b7e302445d31452510`.
<!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:ed85025cfc91c38dbce7a0312936c79b16f8e6a42e4fc5f6492aaf4813d3cc57 -->

- A execução completa alcançou novo jogo e encerrou a introdução por controles normais. Cem retornos reais do analógico confirmaram a direção corrigida. A posição do personagem permaneceu fixa após o spawn; movimento visível, interface, desempenho e integração UWP continuam pendentes.
- A investigação seguinte reproduziu uma falha independente em `CVT.W.S`: arredondar a coordenada 5,5 para 6 seleciona a célula errada e produz subíndice −10 na consulta da área. Truncar para 5 produz índice 10 e retorna área presente com os mesmos dados da execução. A correção está em validação.

<!-- /coderabbit-ai-generated:v1 -->

Esses resultados confirmam a correção da tradução e da normalização isolada. Não estabelecem jogabilidade no Xbox.
