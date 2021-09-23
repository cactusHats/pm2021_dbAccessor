# pm2021_dbAccessor
- PM2021 データベースアクセス用アプリ

# 準備
## 1. addon
- ofxOscを入れる
## 2. ソースファイル(.cpp, .h, .hpp)
- oFプロジェクト内のsrcディレクトリに入れる（既存のファイルは置き換えるか，上書き保存でOK）
- コードの説明
    - oFデフォルトコード
        - main.cpp
        - ofApp.cpp
        - ofApp.h
    - 追加コード
        - define.h：設定値の記述
        - confReader.hpp：設定ファイル読み込み用コード
## 3. 設定ファイル(.conf)
- oFプロジェクトのbin/dataディレクトリにsetting.confを入れる （基本的な設定項目はこのファイルで編集可能）
## 4. MYSQL C++用ライブラリ
- MYSQLのC++ Connectorを使用
- Windowsの場合，下記を参照
    - https://qiita.com/shiro160/private/9ec4cb66fc1b5fb42892
    - https://docs.microsoft.com/ja-jp/azure/mysql/connect-cpp

# 動作説明
- 一定時間ごとにDBを参照しにいく
- DB項目LAUNCHED=FalseのJOBのみを取り込み，処理済みのJOBはLAUNCHED=Trueに更新
- 取り込んだJOBからTYPE_ID（花火の形）のみをOSC通信で花火アプリに送信する

# 参考
- 設定ファイルの読み込みには，下記コードを使用した
- https://github.com/TTRist/confReader