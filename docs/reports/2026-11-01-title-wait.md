<!-- coderabbit-ai-generated:v1 -->

# Investigação da tela cinza: espera na rotina de título

## Observação

Em nova execução do binário Linux de diagnóstico da entrega anterior, com a revisão `e90bc608872921d06f1a0d4d38962b3115160703`, o jogo exibiu o aviso do cartão, avançou com Baixo/X e exibiu a apresentação da Sony. Após Enter/X, a tela passou à área cinza. O disco era `SCUS_974.72` com SHA-256 `d16ad76ba8cdf87695d94b1316d51c1f248959ec0749814c31bb7a2d55d9a2e1`; o código gerado usa o layout específico documentado no [relatório de boot](2026-11-01-original-iop-boot.md).

Breakpoints registraram entrada em `st_opflow` no contador GP 169 e `demo_title` no contador 227. Na tela cinza, uma parada em `opflowChkTitleDemoStart` mostrou `0x01465e40 = 0` e GP 407. Uma nova leitura depois de 20 segundos voltou a mostrar esses valores; uma terceira parada confirmou a pilha `demo_title` → `dispatchGuestBranch` → `opflowChkTitleDemoStart`. Em todas as leituras, o jogo estava rodando em Linux/Xvfb, OpenGL por software e FFmpeg desativado. Nenhum dado da RAM do jogo foi modificado.

A inspeção do C++ gerado para esta versão mostra que `demo_title` consulta o bit 4 de `0x01465e40` e repete a consulta enquanto estiver zero. `opflowOp1End` é uma rotina que marca esse bit, entre outros; isso não demonstra que seja a única forma de marcá-lo ou que sua ausência cause a tela cinza. Uma tentativa de observar sua entrada em uma janela de 15 segundos expirou sem registrar parada no breakpoint. **A linha impressa após a interrupção no log dessa tentativa não é uma entrada comprovada**: o comando de impressão foi executado após o SIGINT do timeout. A causa da espera continua desconhecida.

## Continuidade

Investigar quem deve chamar `opflowOp1End` ou marcar o bit 4 e o estado do agendador, recursos e callbacks envolvidos. Comparar com uma referência antes de modificar o comportamento; não forçar o bit nem remover a espera. O contador GP inalterado durante as leituras é uma observação pontual, não prova de travamento geral. Os arquivos do disco, RAM e código gerado permanecem fora do Git.

A execução não demonstra menu visível, jogabilidade, áudio funcional ou compatibilidade Xbox. O Xbox Series S do usuário tem Dev Mode; ainda faltam backend e pacote Xbox, além de validação com SDK e console.
