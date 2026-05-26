#pragma once

const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="ja">

<head>
    <title>Gamepad DEmo</title>
    <meta charset="UTF-8">
</head>

<body>
    <label>接続状態：　</label>
    <label id="connect">不明</label>
    <div id="disp"></div>
    <script>
        let connect = false;
        let pad = true;
        let ws = new WebSocket('ws://192.168.0.1/ws');
        // ws.binaryType = 'arraybuffer';
        ws.onopen = function (event) {
            connect = true;
            document.getElementById("connect").innerText = "接続";
        }

        //エラー発生
        ws.onerror = function (error) {
            document.getElementById("connect").innerText = "エラー";
        };

        //メッセージ受信
        ws.onmessage = function (event) {

        };

        ws.onclose = function () {
            connect = false;
            document.getElementById("connect").innerText = "サーバーとコントローラーが切断";
        };
        //Create AnimationFrame
        var rAF = window.mozRequestAnimationFrame ||
            window.webkitRequestAnimationFrame ||
            window.requestAnimationFrame;

        //Update
        function update() {
            var pads = navigator.getGamepads ? navigator.getGamepads() :
                (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : []);

            //tekitou
            pads = pads[0];
            if (pads) {
                var but = [];

                //buttons
                let btn = 0;//16bit
                for (let i = 0; i < 16; i++) {
                    let val = pads.buttons[i];
                    let pressed = val == 1.0;
                    if (typeof (val) == "object") {
                        pressed = val.pressed;
                        val = val.value;
                    }
                    btn |= pressed << i;
                    but[i] = val;
                }
                let axes = pads.axes;
                var txt = [];
                txt += pads.id + "<br>";
                for (var i = 0; i < but.length; i++) {
                    if (but[i] == 1) txt += '<input type="checkbox" checked="checked">';
                    else txt += '<input type="checkbox" >';
                }
                for (var i = 0; i < axes.length; i++) {
                    txt += '<br>';
                    txt += axes[i];
                }
                document.getElementById("disp").innerHTML = txt;
                //鯖へ送信
                let joy = new Int16Array(4)
                joy[0] = (Math.abs(axes[0]) < 0.1 ? 0 : axes[0]) * 512;
                joy[1] = (Math.abs(axes[1]) < 0.1 ? 0 : -axes[1]) * 512;
                joy[2] = (Math.abs(axes[2]) < 0.1 ? 0 : axes[2]) * 512;
                joy[3] = (Math.abs(axes[3]) < 0.1 ? 0 : -axes[3]) * 512;
                let data = new Uint8Array(10);
                data[0] = btn & 0xff;
                data[1] = (btn >> 8) & 0xff;
                for (let i = 0; i < 4; ++i) {
                  data[i * 2 + 2] = joy[i] & 0xff;
                  data[i * 2 + 3] = (joy[i] >> 8) & 0xff;
                }
                if (connect) {
                  ws.send(data.buffer);
                }

                if (!pad) {
                    document.getElementById("connect").innerText = "接続";
                }
                pad = true;
            } else if (pad) {
                document.getElementById("connect").innerText = "Joyコンが接続されていません";
                pad = false;
            }
        }

        //Start
        setInterval("update()", 10);

    </script>
</body>

</html>
)rawliteral";