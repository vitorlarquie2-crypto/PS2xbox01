<!-- coderabbit-ai-generated:v1 -->

# Primeiro teste no Xbox Series S

Este alvo é um aplicativo UWP x64 separado para verificar instalação, abertura, tela e controle no Dev Mode. **Ele não contém nem executa Shadow of the Colossus ou o runtime PS2.** Não valida renderização do jogo, áudio, desempenho ou compatibilidade do recompilador. O projeto do jogo ainda depende de portar o host desktop e resolver a espera no título.

## Gerar o pacote

O workflow `Xbox diagnostic package` usa Windows 2022 com MSBuild, SDK Windows 10.0.19041.0 e o workload UWP. Pode ser iniciado pelo GitHub Actions ou por um push que altere `xbox/`. O artefato se chama `PS2Xbox-Diagnostics-x64`. O script também pode ser executado em Windows com esses componentes:

```powershell
./xbox/build-diagnostics.ps1
```

Cada geração cria um certificado de desenvolvimento, assina o APPX e remove a chave do repositório de certificados ao terminar. Somente o certificado público `.cer`, o pacote, dependências e hashes são publicados. Não reutilize essa identidade para uma distribuição comercial. O certificado muda a cada geração; para repetir o teste com outra geração pode ser necessário desinstalar o pacote anterior.

## Instalar e testar

Depois de uma geração bem-sucedida, baixe e extraia o artefato em um dispositivo com navegador e acesso à mesma rede do Xbox. No Dev Home do console, habilite o Xbox Device Portal e abra no navegador o endereço mostrado pelo console. Use o gerenciador de aplicativos para adicionar o APPX principal e as dependências **x64** da pasta Dependencies, caso presentes. O certificado de desenvolvimento acompanha o pacote; a aceitação e instalação precisam ser verificadas no console. Não publique senha ou endereço de acesso do console.

1. Abra `PS2Xbox Diagnostics` e confirme que a identificação da plataforma aparece.
2. Confirme que a tela e os textos estão visíveis sem cortes.
3. Pressione A: a cor deve alternar e a contagem deve aumentar uma vez por pressão.
4. Pressione B: a contagem deve voltar a zero.
5. Mova os dois analógicos, direcionais e gatilhos: os valores exibidos devem mudar.
6. Desconecte e reconecte o controle e confirme a recuperação.
7. Retorne ao Dev Home, reabra o aplicativo e registre o resultado.

Se falhar, anote a etapa, a versão 0.1.0.0 e o código de erro de instalação/abertura. Nenhum arquivo do jogo é necessário para este diagnóstico. <!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:aa6a511a1ff9c3bd216cee54212fd72fa6427de40314c30a97fe4172396b6ee0 -->

A revisão `25365e306b8932e7539e44db47b35136bd8af830` foi compilada e assinada com sucesso no [build Windows](https://github.com/vitorlarquie2-crypto/PS2xbox01/actions/runs/41356258756), com zero erros e zero avisos. O APPX, os hashes e as três dependências x64 foram conferidos. <!-- coderabbit-ai-generated:v1 -->

<!-- coderabbit-ai-generated-section:v1:1fd057115583808eef81b5ff330804f30fd1040ac78c1e7530a08b98d142abd3 -->

O usuário informou que o diagnóstico funcionou sem erros no Xbox Series S em Dev Mode. Esse resultado foi relatado pelo usuário; não houve observação direta do console nesta tarefa.

<!-- /coderabbit-ai-generated:v1 -->

<!-- /coderabbit-ai-generated:v1 -->

Referências: [empacotamento UWP](https://learn.microsoft.com/en-us/windows/uwp/packaging/auto-build-package-uwp-apps), [Xbox Device Portal](https://learn.microsoft.com/en-us/windows/uwp/xbox-apps/device-portal-xbox).
