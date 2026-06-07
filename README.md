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

<img width="986" height="793" alt="スクリーンショット 2026-06-06 100232" src="https://github.com/user-attachments/assets/b05985da-74d9-402a-aa44-7df945f70fbf" />
<img width="986" height="793" alt="スクリーンショット 2026-06-06 100122" src="https://github.com/user-attachments/assets/0a95f5b3-338d-4f3a-aa00-c49ee020e5b7" />

このプログラム作成の経緯

まず、AIでどの程度のことが出来るのか自身の3Dプログラムの勉強を兼ねてGeminiに・・・
「太陽系の光星と惑星間の重力をエミュレートして画像で表示するプログラムを作成できますか？」
と訪ねたら万有引力の法則を用いた１００行程度のプログラムのソースコードが掲示されました。

WindowsのVisual Studioで作成できるC#でのプログラムで太陽系の惑星と月がシミュレートできる
簡単なプログラムまで進み、2Dでの簡単なシュミレーションから3Dによる天体シュミレーションに
変更したり惑星の名前や軌跡の表示など機能追加をしながらコンパイルエラーやら動作エラーを
Geminiに報告しながらデバッグして動作するプログラムが作成出来ました。

自分の理解していないプログラム言語などを実際にコンパイル・実行できる形で勉強できるのは、
非常に有意義で知らない事の連続で大変勉強になりました。

ある程度、プログラムが仕上がってきたので自分の理解しやすいC++のWindowsディスクトップ
アプリケーションに全体の構成を変更してGDI+の描画からDirect2Dの描画を試してみました。

天体シュミレーションも精度の高いと言う4次ルンゲ＝クッタ法を使った軌道計算や火星と木星間の
アステロイドベルトや木星軌道の小惑星の奇妙な動きをシュミレーションすれば面白いとGeminiから
提案されてその度に軌道計算のマルチスレッド化やCUDAを使ったGPU支援などを実装しました。

これ位までGeminiには、楽勝なようでそれほど大きな問題もなく適切なアドバイスとソースコードの
掲示があり比較的、短時間で進みましたが次のステップでDirect3Dによるシュミレーションにしたら
非常に多くの問題がありました。

まず、中途半端にDirect2Dの部分を残してDirect3Dにする場合にDirectXのバージョンを10から11に
しないとうまく動作しないようでGeminiに指摘されるまで悩みました。

次の惑星を3Dでモデリングして3D空間でターゲット・カメラ・光源を指定しての本格的な3D描画に
挑戦しましたが、これが大変でした。まずは、正しく表示されるまでDirect3Dの難解な設定や
コンパイルエラー・実行時のエラーが多発でその度にエラーコードや実行時の画面などをGeminiに
報告・デバッグ指示など受けて、一筋縄には、行きませんでした。

特に衛星の月食や日食をシュミレーションするシャドウマップの作成では、3Dマッピングの矛盾が
浮き彫りになってGeminiにもお手上げ状態になるなど、大変苦労しましたが、非常に面白い会話を
Geminiと行いデバッグ指示・結果報告などを繰り返し問題の根本を見つけるまで楽しかったです。

問題の原因を見つけるとその本質をとても詳しく報告してもらえたので自身の理解を高める事が出来
ました。この点は、非常に素晴らしい経験でAIによる自身の学習が効果的であることを認識しました

昨今のAIによるプログラム支援の有効性は、一発で欲しい機能のプログラムを作成してもらうのでは
無く、欲しい機能の作成の理解を助けてもらえると考えると非常に有効だと思いました。
