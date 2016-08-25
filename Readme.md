# MayaGameEngineInterface
Copyright (c) 2016 GANBARION Co., Ltd.

MayaのViewport2.0をオーバーライドして独自の描画エンジンを用いてMaya上にレンダリングを行うためのMayaと描画エンジンとのインターフェースです。

- CEDEC2016 「[今世代向けプロダクションにおけるアセット製作のための、Mayaへのエンジン組み込み](http://cedec.cesa.or.jp/2016/session/ENG/5635.html)」
- AREA JAPAN コラム「[Mayaにゲームエンジンを組み込んでみよう！～効率的なアセット製作環境を目指して～](http://area.autodesk.jp/column/tutorial/maya_game_engine/)」

## ビルド方法
### 必要なもの
- Visual Studio 2015
- Autodesk® Maya 2016 or 2017
- Autodesk® Maya 2016 - Developer Kit (Maya2016のみ)
	- [Autodesk App Store](https://apps.autodesk.com)よりイントールしているMayaのバージョンに合わせてダウンロード
    - 取得後展開し、Maya2016インストールディレクトリにコピーしてください。

### ビルド
- MayaCustomViewport.slnを開く
- Mayaをデフォルトパス以外のパスにインストールした場合はプロジェクトの追加のインクルードディレクトリ、追加のライブラリディレクトリをインストールパスに合わせて修正してください。

## 実行方法
### 起動方法
- マイドキュメント/Maya/plug-insにMayaCustomViewportディレクトリを作成しassets内のファイルをコピー
    - 最終的にplug-ins/dx11Shader, plug-ins/shaderとなるようにする
- Maya環境のscriptsにscriptsディレクトリ以下をコピー
- Mayaを起動
- プラグインマネージャよりロード
- モデルパネルの「レンダラ」メニューからMayaCustomViewportを選択

### マテリアルと描画エンジンのリンク方法
- dx11Shader以外のシェーディングノードはすべて白単色で描画される
- dx11Shaderを作成し、シェーダファイルにplug-ins/dx11Shader/MayaShader.fxを指定することでシェーダを切り替えできる

## ライセンス
MIT License.

## 使用ライブラリ
- [picojson](https://github.com/kazuho/picojson)
- FXAA 3.11

## リファレンス
- [Killzone Shadow Fall: Creating Art Tools For A New Generation Of Games](http://www.slideshare.net/guerrillagames/killzone-shadow-fall-creating-art-tools-for-a-new-generation-of-games)
