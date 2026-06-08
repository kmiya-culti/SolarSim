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

#### このプログラム作成の経緯

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

まず、中途半端にDirect2Dの部分を残してDirect3Dにする場合にDirectXのバージョンを11から11.1に
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

#### プログラムソースコードの詳細な説明

まず、SoloarSim.hのコンパイル時のオプション設定です
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.h#L24-L44
出来るだけ追加していった機能をコンパイルオプションで残しています。

最初が描画を行うGUIの選択です。GDI+、Direct2D、Direct3Dで描画速度が劇的に速くなっていきます。
実際に違いが現れるのは、ASTEROID_BELTやTROJAN_ASTEROIDS、SATURN_RINGSなどで小惑星を表示した場合で
数千から数十万の小惑星の描画には、DirectXの力を借りないと従来のGDIでは、非常に苦しいです。
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L2959-L2961

次にMULTI_THREADで軌道計算などをメインCPUのマルチスレッド化します
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L1847-L1852
これは、Geminiの提案でC++17の機能ですよね・・・

次にRK4_TESTで4次ルンゲ＝クッタ法を用いた軌道計算の実装です
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L1725-L1726
これもGeminiからの提案で実装しましが、マルチスレッド化で見にくくなってしまいました。
実際の計算は、４倍遅くなりますが同じタイムステップでも精度が数万倍に跳ね上がるそうです・・・

これでMULTI_THREADとDIRECT2D_VIEWで小惑星の軌道計算と描画が可能になりましたが次にCUDAによりGPU支援です
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/kernel.cu#L15
この実装前に厄介なデータ構造の変更を提案されました。
「CUDA（GPU）の性能を引き出すためには「要素ごとの配列（SoA: Structure of Arrays）」への構造変更が非常に重要です」
だそうで現在位置のデータ、現在の速度のベクトルデータなどを構造体で一括管理していたのをすべて配列に置き直しました
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L88-L91
この作業は、ソースコードの見た目が一昔前のようになり困惑しましたが、後の実装で必要性を実感しました
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L1701-L1722
軌道計算に必要なデータは、沢山ありますが・・・
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L1930-L1933
計算結果で表示に必要なのは、これだけです。毎回GPU<->CPUでのメモリ転送が必要なのがボトルネックになるそうです。
さらにGPU側で必要なメモリ量なども事前に見積もって
「現在のシミュレーターの規模（数万〜数十万天体）であれば、メモリ不足を心配する必要はほとんどありません」
とのことで安心して実装しました。

次のステップでGeminiから[GPUで計算した座標データを、メインメモリに戻さずそのまま描画バッファとして利用できます]
その提案があり興味がわいたのですが、これからが、Direct3Dへの変更に進み非常にややこしくなりました。
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/kernel.cu#L238
の提案は、簡単なのですが、作成される頂点シューダーをどのように扱うのかさっぱりでした。

とりあえずDirect3Dのイニシャルなどを行ったら今まで表示されていたDirect2Dの描画が動作しません。
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L235-L268
結局、Direct2Dで使用するレンダーターゲットの作成方法など色々と試したら
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.h#L112
でDirect2Dのバージョンを2D1_1に上げてやる必要があったようでした

さらにややこしいのがhlslなる別のソースコードを別にコンパイルする必要があるようでネットで検索してもサンプルが少なく
どのような形で利用するのかさっぱりでした。
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/planet.hlsl#L17-L44
色々と探した結果、プログラム内でコンパイルする方法と
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L670-L702
fxc.exeなるコンパイラでコンパイルしてヘッダファイルの形で読み込む方法がありました
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L904-L924

なんとなく頂点シューダーとピクセルシューダーがどのように動作するかのイメージを掴むのに苦労しましたが、動作してしまえば簡単で
天体シミュレータの座標から表示する画面の座標変換を行う頂点シューダーとその点の色を決定するピクセルシューダーをコンパイルして
それを目一杯並列で動作される訳ですね・・・相関の無い部分なので並列処理に持ってこいですね。

ここまでで表示をDirect3D上のDirect2Dで行い、小惑星のみCUDAの支援で軌道計算と座標変換を行って3Dの並列処理で画面に表示する
ことが出来ました。次のステップでDirect2Dで簡易的に表示していた惑星を本格的な球体のモデリングを行って表示することに挑戦しました。
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L1025-L1042
球体のモデリングは、Geminiに掲示してもらって使用したのですが、これが最後まで悩みの種になり最終的なソースは、以下のようになりました。
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L554-L602
多少、苦労の跡が残っていますが、後に惑星の画像のテクスチャを貼り付けた時に９０度傾いて表示される。表裏が反転して表示されるなど
１つ機能を追加するとおかしな動作がついてまわりました。

最終的には、Direct3Dが左手座標でターゲット・カメラ・光源を指定して画面に表示した場合に画面の上が+Y軸、右が+X軸、奥行きが+Z軸で
手前が-Z軸になります。これを一般的な球体でモデリングしてリンゴをテーブルに置いた場合などは、上が+Y軸でモデリングしないとリンゴのヘタ
が上に表示されません。しかし天体モデルで考えると地球の公転面をXY面に展開すると北極が-Z軸なります。+Yだと９０度傾くことになります。
従って最終ソースコードの
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L563
でZ軸で上下に配置しているのですが
https://github.com/kmiya-culti/SolarSim/blob/0e27d627cc5e9e18a07204d14be486cefe495b62/SolarSim/SolarSim.cpp#L580-L581
のようにテクスチャを-Zを頂点に強制しています。最後までGeminiには、-Zを頂点にモデリングしたコードがうまく作成できないようでした
