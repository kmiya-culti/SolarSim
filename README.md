この天体シミュレータプログラムは、Google Geminiと共に色々と試行錯誤しながら作成しました。
特にWindowsのDirectXの2D/3Dによる描画には、サンプルとしての有用性があると思います。
GDI+やDirect2Dで描画する部分もコンパイルオプションとして残しています。

<img width="328" height="264" alt="スクリーンショット 2026-06-06 100232" src="https://github.com/user-attachments/assets/b05985da-74d9-402a-aa44-7df945f70fbf" />
<img width="328" height="264" alt="スクリーンショット 2026-06-06 100122" src="https://github.com/user-attachments/assets/0a95f5b3-338d-4f3a-aa00-c49ee020e5b7" />

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
これは、Geminiの提案でC++17の機能ですよね・・・また、Geminiからの提案で「構成プロパティ・C/C++・コード作成」の
「拡張命令セットを有効にする」を Advanced Vector Extensions 2 (X86/X64) (/arch:AVX2)「浮動小数点モデル」をFast (/fp:fast)
が有効とのことで試したら約1.3倍程速くなりました。現在のプロジェクトでそのように設定していますが自身のCPUに合わせて変更してください。

次にRK4_TESTで4次ルンゲ＝クッタ法を用いた軌道計算の実装です
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/SolarSim.cpp#L1725-L1726
これもGeminiからの提案で実装しましが、マルチスレッド化で見にくくなってしまいました。
実際の計算は、４倍遅くなりますが同じタイムステップでも精度が数万倍に跳ね上がるそうです・・・

これでMULTI_THREADとDIRECT2D_VIEWで小惑星の軌道計算と描画が可能になりましたが次にCUDAによりGPU支援です
https://github.com/kmiya-culti/SolarSim/blob/585e7ac5642bd674f0f9d44ff9466a1fb5e4e9bf/SolarSim/kernel.cu#L15
この実装前に厄介なデータ構造の変更を提案されました。
「CUDA（GPU）の性能を引き出すためには「要素ごとの配列（SoA: Structure of Arrays）」への構造変更が非常に重要です」
だそうで現在位置のデータ、現在の速度のベクトルデータなどを構造体で一括管理していたのをすべて配列に置き直しました
https://github.com/kmiya-culti/SolarSim/blob/7e4ccfc73b91057b1db6b6cdd03b35208fc7ed31/SolarSim/SolarSim.cpp#L36-L39
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

この時に行ったデバッグでGeminiが提案した方法で面白いのは、ピクセルシューダーで本来なら色を返すのですが、
https://github.com/kmiya-culti/SolarSim/blob/b93179ab81baad9d66133568bf79bfd0f74c6be0/SolarSim/planet.hlsl#L41-L44
これをreturn input.Posのようにして色の変わりに座標を返すようにして表示される球体の画像を送り返す方法でした。
右上が赤いからX軸は、正しいが下方が緑なのがおかしいなどの判断を行ってデバッグしました。画像でデバックできるのがすごいですね。
これも賢いデバッグ方法でhlslのデバッグが、非常に困難なのでこのような工夫が必要なのでしょう・・・

<img width="328" height="264" alt="image" src="https://github.com/user-attachments/assets/c3640708-81fe-488c-8854-bac8504e5b30" />

次に日食や月食などの衛星や惑星の影を表現したいとGeminiにお願いしました。
まずは、概略でシャドウマップを作成してカメラから見た視線を太陽から見た視線に置き換えたテクスチャを作成しなさいとのことで
おもしそうなので適当にちゃっちゃと作成して実行してもさっぱり効果がわかりませんでした。
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L1195-L1223
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L2207-L2223
自分で作成した部分をGeminiに添削してもらってとりあえず動作しているように思っても効果を確認する方法がわかりません
「木星の衛星イオはものすごい頻度で木星に影を落とし、また木星の影に隠れています」との助言をもらっても確認できません

結局、シャドウマップテクスチャをダイレクトに画面右下に表示するデバッグ画面を作成するように薦められて実装しました。
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L2740-L2764
これが実に分かりやすい確認方法で太陽視点の惑星の重なりを簡単に確認でき間違ったサンプラーの設定などを修正できました。
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/planet.hlsl#L168-L179
SampleCmpLevelZeroの扱いが少しややこしかったです

次にテクスチャの貼り付けのテストで土星の環を作成しました
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L1063-L1088
このあたりは、得に問題もなくGeminiの提案を少し変更する程度で実装することができました。ピクセルシューダーで実際に貼り付ける部分が
癖があって面白い部分ですね
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/planet.hlsl#L323-L348

なんとなくテクスチャ回りの実装が解れば、惑星のテクスチャ画像をネットでさがして2Dのテクスチャの貼り付けるだけなのでそれなりに
実装しましが画像で惑星が表示されると自転軸の傾斜や自転速度など今まで無視していたデータ必要になって結構、大変でした。
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/PlanetTexture.cpp#L44-L57
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L1559-L1588

見た目がそれなりになってきたのですが、冥王星の動きを拡大するとなにやらゴソゴソしています。天体のデータは、すべてdoubleの浮動小数点演算
で計算していますが、Direct3Dに渡す座標データをfloatに縮小する必要があります。太陽から冥王星までがE+12程あり隣の衛星は、E+5程なので
12-5=7桁ほど差があります。これがfloatの7桁の精度にひっかかってガタガタしてしまいます。これをGeminiに相談すると面白い解決方法を提案されました。
「解決策：【王道】カメラ相対座標系（Floating Origin / 宇宙の中心をカメラにする）」とのことで王道だそうです。
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L2410
実装は、簡単で画面に表示するときの中心位置の惑星座標を引き算するだけです。注意点は、引き算は、doubleの時でfloatにしてからの引き算は、
意味が無いと言うことこで実際に実装したときにきっちり間違ってしまいGeminiに指摘されました。floatでは、0.0に近い値にして計算すれば
精度を上げることが出来る。それを実現するのに引き算を行う・・・天体シミュレータの王道だそうです。

最後にDirect3D on 2Dで残っていた軌跡の表示
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L2576-L2585
惑星横に惑星名の表示を2Dテクスチャにして貼り付け
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L319-L371
https://github.com/kmiya-culti/SolarSim/blob/0b78a21535468cd93bf7a2dc50fb0e65f9a20d48/SolarSim/SolarSim.cpp#L2660-L2679
を行って画面左上のデバッグ情報のみをDirect2Dにしました。

どちらも自力で作成して問題をGeminiにデバッグしてもらって修正で比較的簡単に実装できました。
これは、癖になりそうなくらい自力デバッグの労力が少なくなる経験でした。イージーな設定ミスやパラメータ間違いなど楽勝で見つけてくれます。

土星の環のピクセルシューダーでの1Dテクスチャの貼り付け
https://github.com/kmiya-culti/SolarSim/blob/d327d7122e34a872b82cee5b97603e667befc7d7/SolarSim/planet.hlsl#L309-L310
惑星名の表示のピクセルシューダーでの2Dテクスチャの貼り付け
https://github.com/kmiya-culti/SolarSim/blob/d327d7122e34a872b82cee5b97603e667befc7d7/SolarSim/planet.hlsl#L375-L376
最初は、単純に土星の環のピクセルシューダーをコピペしたのですが、気が付きませんがGeminiに指摘されれば、イージーなミスだと思いました。

この様なDirect3Dでの細かな設定も表示の異常を引き起こしますが、なかなか気づけません。
https://github.com/kmiya-culti/SolarSim/blob/d327d7122e34a872b82cee5b97603e667befc7d7/SolarSim/SolarSim.cpp#L1125-L1150
惑星名の文字列をDirectWriteで書き出しますが、アンチエイリアスとテクスチャの拡大・縮小で相当表示が崩れます。

これも設定項目が多すぎですよね・・・・
https://github.com/kmiya-culti/SolarSim/blob/d327d7122e34a872b82cee5b97603e667befc7d7/SolarSim/SolarSim.cpp#L321-L350
Formatの指定をテクスチャがDXGI_FORMAT_R8G8B8A8_UNORMでビットマップをDXGI_FORMAT_B8G8R8A8_UNORMで間違っていました。
これなんかもぱっと見では、解りませんでした・・・せめて表示して色がおかしいなどの症状だとうれしいのですが全く動作が止まってしまいます。
ソースコード内のコメントは、Geminiが作成した物で(サイズは 256x64 程度で十分です)などは、当初1024*256で指定していました。

人による簡単なコピペで起こすようなミスや異常な表示、エラーコードを吐いて処理を中断などのデバッグは、GeminiなどのAIにとっては、
「はいはい・・それは、これですね！」程度で指摘してもらえます。また、論理的なミスなどは、問題の原因を見つけるとその本質をとても詳しく
分析して報告してもらえたので自身の理解を高める事が出来ました。この点は、非常に素晴らしい経験でAIによる自身の学習が効果的ですね。

昨今のAIによるプログラム支援の有効性は、一発で欲しい機能のプログラムを作成してもらうのでは、無く、欲しい機能の作成の理解を助けて
もらえると考えると非常に有効だと思いました。
