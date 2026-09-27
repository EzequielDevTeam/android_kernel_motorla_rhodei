# Tutorial: instalando o kernel com KernelSU (Moto G62 5G)

> Leia TUDO antes de começar. Siga na ordem. Dúvidas? Abra uma Issue.

## 0. O que é cada coisa (sem jargão)

- **Image.gz**: o kernel novo com KernelSU, que você baixa aqui na aba **Releases**.
- **boot.img**: arquivo que o celular usa para ligar. Vamos pegar o SEU original e só trocar o kernel dentro dele.
- **Manager (APK)**: o aplicativo do KernelSU, onde você libera ou nega root por app.
- **`fastboot boot`**: testa sem gravar nada (seguro). **`fastboot flash`**: grava de vez.

## 1. O que você precisa

1. PC com Windows ou Linux + [platform-tools](https://developer.android.com/tools/releases/platform-tools) (tem `adb` e `fastboot` dentro).
2. **Bootloader desbloqueado** (Motorola: pede código no site da Motorola. ATENÇÃO: zera o aparelho e perde garantia).
3. O **`boot.img` original** da sua ROM atual (extraia do firmware stock do seu modelo/variante, ex: rhodec).
4. O **`Image.gz`** da nossa Release.
5. O APK **KernelSU Manager v3.3.0** ([lançado no GitHub do KernelSU](https://github.com/tiann/KernelSU/releases)).
6. Bateria acima de 50% e backup das suas fotos/contatos.

## 2. Tire o Magisk do caminho (importante!)

1. Abra o app do **Magisk** > **Desinstalar** > **Restaurar imagens**.
2. Reinicie e confira que o Magisk sumiu.
3. Motivo: KSU + Magisk juntos brigam e banco detecta mais fácil.

## 3. Monte o novo boot.img (no PC)

1. Baixe o **Android Image Kitchen (AIK)** e descompacte.
2. Copie seu `boot.img` original para a pasta do AIK e rode o script de desempacotar.
3. Na pasta `split_img`, ache o arquivo do kernel (chama `boot.img-kernel`) e **substitua** pelo nosso `Image.gz` (renomeie para o mesmo nome).
4. Rode o script de reempacotar. Vai sair um `image-new.img`. Renomeie para `boot-ksu.img`.

## 4. Teste SEM gravar (seguro)

1. Desligue o celular. Segure **Volume - + Power** até entrar no fastboot.
2. No PC: `fastboot devices` (tem que listar seu aparelho).
3. Teste: `fastboot boot boot-ksu.img`
4. O celular liga normal. Instale o APK do Manager v3.3.0 e abra: tem que mostrar **"Trabalhando"** (verde).
5. Teste root com um app de terminal. **Não gostou / não ligou?** Reinicie e está tudo como antes. Nada foi gravado.

## 5. Gravação definitiva (só se o teste passou)

1. Volte ao fastboot.
2. Grave: `fastboot flash boot boot-ksu.img`
3. `fastboot reboot`

## 6. Depois de ligar

1. Abra o Manager, **não libere root para banco/jogo**, só para apps de confiança.
2. Guarde seu `boot.img` original num lugar seguro (é seu paraquedas).

## Socorro!

- **Não liga / bootloop**: entre em fastboot e grave o `boot.img` original de volta: `fastboot flash boot boot-original.img`.
- **Banco detectando**: não libere root pra ele, use perfil dedicado. Relate na Issue do seu aparelho.
