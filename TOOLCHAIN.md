# Toolchain Windows CE isolada

## Identificação confirmada no upstream

Repositório estudado: [enlyze/ghcr-windows-ce-build-environment](https://github.com/enlyze/ghcr-windows-ce-build-environment).

- README do repositório: CeGCC **9.3.0**, com alvos ARM e i386.
- `Dockerfile`: base `debian:12-slim`.
- `Dockerfile`: `ARG target_arch` e `ENV PATH=/opt/cegcc-${target_arch}/bin:$PATH`.
- `Dockerfile`: execução de `./build_cf.sh /opt/cegcc-${target_arch} ${target_arch}-mingw32ce`.
- workflow upstream: matriz `target_arch: [arm, i386]`.
- para a variante ARM, o prefixo do alvo é `arm-mingw32ce` e a localização esperada é `/opt/cegcc-arm/bin`.
- compilador esperado: `/opt/cegcc-arm/bin/arm-mingw32ce-gcc`.
- ferramentas esperadas: `arm-mingw32ce-objdump`, `-nm`, `-strings`, `-windres`, `-ar`, `-ranlib`, `-strip`.

A página do pacote ARM registra a imagem publicada:

`ghcr.io/enlyze/windows-ce-build-environment-arm:main-64dda99`

Digest exibido pelo GHCR:

`sha256:22ea23d697accaab2b98fb12d81ccc1faade90f9fe490f554ae7c5c194dfe27c`

O workflow local fixa a tag `main-64dda99` para tornar o build reproduzível em relação à versão observada. A imagem sem sufixo ARM citada originalmente pelo usuário não identifica explicitamente qual arquitetura; por isso foi usada a variante ARM publicada.

## Bibliotecas e COREDLL

O Dockerfile constrói os subprojetos `mingw` e `w32api`, portanto a imagem deve conter headers e import libraries MinGW/Windows CE. A documentação CeGCC informa que o alvo nativo `arm-wince-mingw32ce` é o apropriado para a API CE e que o exemplo mínimo pode depender somente de `coredll.dll`; ela também lista a forma histórica do compilador e do `objdump`.

A presença exata de `libcoredll.a`/arquivo equivalente e os imports gerados pela versão ENLYZE não foram presumidos: o workflow registra `gcc -dumpspecs`, compila sem copiar DLLs e inspeciona `objdump -p`. O build só será aceito depois de conferir os imports reais. Se aparecer `mingwm10.dll`, `libgcc` ou outra DLL de runtime, isso será documentado como requisito separado; nada será distribuído automaticamente ao GPS.

## Target ARM conservador

O workflow testa, dentro da própria imagem, as flags nesta ordem:

1. `-march=armv4t -marm -mthumb-interwork`
2. `-march=armv4 -marm`

O primeiro conjunto é preferido porque cobre ARMv4T/interworking sem habilitar ARMv5+, Thumb-2, NEON ou VFP obrigatório. A escolha só é feita se o compilador presente no container aceitar a combinação; caso contrário o script falha ou usa a segunda combinação explicitamente registrada. Não há fallback para ARMv5/ARMv7.

Flags comuns:

`-O2 -mwin32 -D_WIN32_WCE=0x0600 -D_WIN32_IE=0x0400 -DUNICODE -D_UNICODE`

`-mwin32` e `_WIN32_WCE` seguem a documentação do CeGCC para o alvo nativo CE. O linker padrão do target `arm-mingw32ce` deve fornecer o startup Windows CE; o resultado é validado, em vez de alterar o cabeçalho manualmente.

## Limitações conhecidas

- A imagem e o projeto têm publicação observada há mais de dois anos; não foi afirmado que cada binário gerado seja compatível com este GPS sem teste estrutural e depois teste controlado no aparelho.
- O container é Linux/Debian; ele gera PE/COFF para CE, não ELF, mas o workflow falha se `file`/`objdump` não confirmarem ARM e Windows CE/9.
- ARMv4 versus ARMv4I não é sempre distinguível apenas pelo cabeçalho PE. A compatibilidade de instruções é controlada pelas flags e deve ser confirmada no log do compilador.

## Fontes consultadas

- [README do ambiente GHCR](https://github.com/enlyze/ghcr-windows-ce-build-environment#readme)
- [Dockerfile upstream](https://github.com/enlyze/ghcr-windows-ce-build-environment/blob/main/Dockerfile)
- [Workflow upstream](https://github.com/enlyze/ghcr-windows-ce-build-environment/blob/main/.github/workflows/docker.yml)
- [CeGCC build fork](https://github.com/enlyze/cegcc-build)
- [CeGCC detalhes de macros, bibliotecas e linker](https://cegcc.sourceforge.net/docs/details.html)
- [CeGCC uso do alvo nativo mingw32ce](https://cegcc.sourceforge.net/docs/using.html)
- [Pacote GHCR ARM e digest publicado](https://github.com/enlyze/ghcr-windows-ce-build-environment/pkgs/container/windows-ce-build-environment-arm)
