Public Key Infrastructure (PKI)
---

# Symmetric Encryption(對稱式加密)

加/解密使用同一把 key, 又稱為單一或私密鑰匙(secret-key), 或傳統加密系統

+ Pros
    - Fast
    - 如果使用足夠大的 key, 將難以破解

+ Cons
    - 需要有一個安全性機制將 Key 安全性的分送至交易的雙方
    - 無法提供識別能力, 無論是誰, 只要有 key 就能解密
        > I am KING if I has the KEY

+ Algorithm
    - DES
    - TDES
    - AES


# Asymmetric Encryption (非對稱式加密)

加解密使用 key-pair, 一個 key (Public) 用來加密, 另一個 key (Private) 用來解密

+ Pros
    - Public key 可以公開分送
    - 提供私密性, 驗證性與不可否認性等服務
        > Public key 無法解密, 即使第三方有 Public Key, 也無法解密內容 (只有 Private Key 可以解密)

+ Cons
    - Bad performance

+ Algorithm
    - RSA
    - Deffie-HellmanKey Exchange
    - Elliptic Curve (ECC)

# Digital Signature (數位簽章)

單向不可逆, 通常使用 Hash Function

+ 雜湊函數特性
    - 單向的映射函數(One way transformation), 無法由輸出反推其原輸入值 (不可反逆)
    - 抗碰撞性(collision resistance)
        1. 雜湊值需隨明文改變而變動
        1. 很難找出二個不同的文件具有相同的雜湊值
        1. 常被稱為 "數位指紋" (Digital Fingerprint)
    - 擴張性(Diffusion)
        1. 指明文中的任何一個小地方的變更都將會擴散影響到密文的各部份


+ Algorithm
    - MD2、MD4、MD5
    - SHA (Secure Hash Algorithm)
    - RIPEMD-160

