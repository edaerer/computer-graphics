# 🐠 Balık Yüzme Simülasyonu

**OpenGL** kullanılarak oluşturulmuş gerçek zamanlı bir 3B su altı sahnesi. Özellikler:
- Yüzen bir palyaço balığı
- Animasyonlu bir ahtapot
- Sallanan bir korsan gemisi
- Dokulu bir okyanus odası
- Birinci şahıs ve üçüncü şahıs kamera görünümleri
- Açılıp kapanabilen kullanıcı arayüzü (UI)

## 🔧 Özellikler

- **Kamera Modları**
  - `C` tuşuna basarak birinci şahıs (balığı takip eder) ve üçüncü şahıs (WASD + fare) görünümleri arasında geçiş yapabilirsiniz.
- **Animasyonlar**
  - Balık dairesel bir yol üzerinde yüzerek yukarı-aşağı hareket eder.
  - Ahtapot ve gemi yerinde hafifçe sallanarak gerçekçilik kazandırır.
- **Ortam**
  - Bir küp şeklindeki oda, su altı atmosferi için okyanus resmiyle kaplanmıştır.
- **Aydınlatma**
  - Derinlik hissi için basit Phong ışıklandırma uygulanmıştır.

## 🎮 Kontroller

| Tuş         | İşlev                                      |
|-------------|---------------------------------------------|
| `W` `A` `S` `D` | Üçüncü şahıs kamera ile hareket             |
| `C`         | Birinci/Üçüncü şahıs kamerası arasında geçiş |
| `ESC`       | Uygulamadan çıkış                            |
| Fare        | Üçüncü şahıs kamerasını döndür               |

## 🖼️ Varlıklar (Assets)

- `models/` — Balık, ahtapot ve gemiye ait FBX modelleri.
- `textures/ocean.jpg` — Okyanus odası dokusu.
- `fonts/Arial.ttf` — UI arayüzünde kullanılan yazı tipi.
- `shaders/` — GLSL vertex ve fragment shader dosyaları.

## 🛠️ Gereksinimler

- C++17 veya daha yenisi
- [GLFW](https://www.glfw.org/)
- [GLAD](https://glad.dav1d.de/)
- [GLM](https://github.com/g-truc/glm)
- Assimp (FBX model yükleme için)
- FreeType (yazı tipi çizimi için)

## 🔄 Derleme Talimatları

1. **Bu repoyu klonlayın**

    ```bash
    git clone https://github.com/edaerer/computer-graphics.git
    cd computer-graphics
    ```

2. **Makefile ile derleyin**

    ```bash
    make
    ```

3. **Simülasyonu çalıştırın**

    ```bash
    ./fish_swim
    ```
