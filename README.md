# m5-soracom-navi
mini car navigation system.<br />
用意するもの<br />
<ul>
<li>M5stack gray/basic</li>
<li>マイクロSDカード</li>
<li>soracom 3G module</li>
<br />
使い方<br />
<ol>
  <li>google geolocation APIのAPI keyを取得する</li>
  <li>SDカード直下にapikey.txtという名前でgoogle geolocation API keyを記入し、保存しておく</li>
  <li>arduinoにm5-soracom-navi.inoを書き込む</li>
  <li>地図データをあらかじめ国土地理院からダウンロードしておくと表示が早い</li>
  <li>タイルダウンロードが終わるまで何も表示されないので不安な人はシリアルモニタで確認してください。</li>
</ol>
