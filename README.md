この天体シミュレータプログラムは、Google Geminiと共に色々と試行錯誤しながら作成しました。
特にWindowsのDirectXの2D/3Dによる描画には、サンプルとしての有用性があると思います。
GDI+やDirect2Dで描画する部分もコンパイルオプションとして残しています。

このプログラムのライセンスは、MITライセンスで公開します。
その他のライブラリなどは、それぞれのライセンスに依存します。

開発環境: Visual Studio 2026 / C++20
ビルド手順: Visual Studioでソリューションファイルを開き、ビルドしてください
操作方法：プラグラムソース内のキー操作、マウス操作を参照してください

コンパイルオプションによりNVIDIA CUDA Toolkitを使用します。
The NVIDIA CUDA Toolkit is used through compilation options.

NVIDIA CUDA Toolkit 13.2
https://developer.nvidia.com/cuda/toolkit

惑星などの位置情報は、NASA Horizons Systemにより取得しました。
The positional information for planets and other celestial bodies was obtained using the NASA Horizons System.

Horizons System
https://ssd.jpl.nasa.gov/horizons/app.html#/

惑星などのテクスチャ画像データは、下記より取得し縮小して使用しました。
The texture image data for planets and other elements was obtained from the source below, scaled down, and used for this purpose.

Solar Textures
https://www.solarsystemscope.com/textures/
Attribution 4.0 International
You may use,adaptmand share these textures for any purpose,even commercially.
https://creativecommons.org/licenses/by/4.0/

衛星などのテクスチャ画像データは、下記より取得し縮小して使用しました。
The texture image data for satellites and other objects was obtained from the source below, scaled down, and used for this purpose.

Image or texture
https://science.nasa.gov/science-org-term/image-or-texture/

謝辞 / Acknowledgments
本プロジェクトの3Dグラフィックスパイプラインの構築、および複雑な座標系のデバッグにおいて、AIアシスタント（Google Gemini）を共同開発パートナーとして活用しました。
Special thanks to Google Gemini, which acted as a co-pilot in debugging complex 3D coordinate spaces and optimizing the Direct3D 11 rendering pipeline.*
