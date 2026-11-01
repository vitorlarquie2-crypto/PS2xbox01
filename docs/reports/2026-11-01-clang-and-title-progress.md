<!-- coderabbit-ai-generated:v1 -->

# Correção de Clang e avanço do título

## Reparo verificado

O commit `149ecb584490a8bc6287584a6066fd8b2bef3547` corrige a ligação Release dos dois executáveis de captura. A biblioteca `ps2_runtime` usa IPO; esses consumidores não habilitavam LTO. Clang não conseguia ler os membros LLVM bitcode do arquivo estático e encerrava com `file format not recognized`. Ambos agora usam o mesmo helper de otimização do runtime.

O erro original foi reproduzido compilando o consumidor sem IPO e retirando LTO da ligação ao mesmo arquivo estático. Com o reparo, os dois executáveis ligaram e passaram seus testes. A suíte C++ Clang Release passou **462/462 casos**, além dos **2/2 testes de captura**. <!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:c0bb4057e3238dc4953dc9c41c77f83a7bfee5b50aee13f30f7071390971ed7d -->

A geração GCC Debug permaneceu sem LTO. O [CI remoto](https://github.com/vitorlarquie2-crypto/PS2xbox01/actions/runs/41361156163) também confirmou compilação e 462/462 testes com Clang, incluindo a ligação dos executáveis de captura; o job GCC concluiu com sucesso. O job Windows ainda estava em execução no momento desta verificação.

<!-- /coderabbit-ai-generated:v1 --> Clang/LLVM 21.1.8, CMake 4.2.3, Ninja 1.13.2; Studio, debug UI e FFmpeg desativados na validação local.

## A espera anterior foi superada

O mesmo runner Linux da entrega anterior foi executado, sem recompilação ou alteração de RAM pelo debugger. Down/X selecionou Continue no diálogo do cartão; Return foi enviado pela entrada normal. O contador GP não é uma medida de FPS.

- Primeira execução: GP401 →453 →550. Um breakpoint real registrou `opflowOp1End` chamado por `demo_op1` no GP550, com flag zero. Após executar a rotina original com `finish`, a flag `0x01465e40` passou a `0x11`. O servidor X encerrou em seguida, interrompendo a verificação da próxima tela.
- Segunda execução: `opflowOp1End` no GP401; `opflowChkTitleDemoStart` chamado por `demo_title` no GP402 leu `0x11`. O campo de carregamento `0x014657fc` era zero. Mais tarde, a retomada de `demo_title` em `0x01403390`, GP469, confirmou execução do laço posterior à inicialização da animação de menu. Esse último ponto não comprova a renderização da animação.

As observações superam a hipótese de que a flag nunca é liberada, levantada nos [rastreamentos anteriores](2026-11-01-op1-runtime-trace.md). Não houve necessidade de forçar o bit ou remover a espera. A correção de compilação não causou esse avanço: o runner permaneceu idêntico, SHA256 `6a1bccdcede2764306ccf049e2257504164a7a921bcc425b81e906ef26fcc2f2`. A conclusão sem entrada do usuário não foi testada.

## Limitações atuais

A Preview Chromium ainda mostrou um retângulo cinza; o menu visível, cenário, áudio e jogabilidade continuam sem validação. O aplicativo UWP de diagnóstico funcionou no Series S segundo o usuário, mas não contém o runtime nem o jogo. A integração gráfica e de execução Xbox permanece por implementar. <!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:3972df9f2a1bbe1c2dce6043f7dc7c16db5cc57f12b50953cbbe58253b035be0 -->

Os dados do jogo, código gerado e logs/RAM privados permanecem fora do Git. A segunda execução foi encerrada deliberadamente pelo GDB no GP573, com flag0x11 e carregamento pendente zero; sessão de diagnóstico saiu0. A área cinza permaneceu sem reparo demonstrado.

<!-- /coderabbit-ai-generated:v1 -->
