# WINCE_BUILD

Projeto local e reproduzível para compilar aplicativos Windows CE ARM com a imagem pública `ghcr.io/enlyze/windows-ce-build-environment-arm`.

O projeto foi preparado sem instalar toolchain no host, sem criar repositório, sem fazer push e sem modificar `D:`. A fonte copiada para `src/main.c` tem o mesmo SHA-256 da fonte original em `BUILD/src/main.c`.

O workflow usa a tag imutável observada no GHCR (`main-64dda99`) e roda em container Linux. Ele:

1. verifica `arm-mingw32ce-gcc` e registra versão/specs;
2. testa flags ARMv4T/ARMv4 conservadoras;
3. compila `hello_tardis.exe`;
4. somente se o hello passar, compila `Tardis.exe`;
5. inspeciona `file`, `objdump -f` e `objdump -p`;
6. falha quando o resultado não for identificado como PE ARM ou não declarar Windows CE/9;
7. publica ambos como artifacts do workflow.

Nenhum runtime/DLL é copiado automaticamente. Os imports reais e qualquer DLL CeGCC exigida devem ser obtidos dos logs antes de considerar o binário executável no GPS.
