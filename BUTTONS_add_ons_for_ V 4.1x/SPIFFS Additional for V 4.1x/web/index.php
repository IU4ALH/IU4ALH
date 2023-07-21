<!DOCTYPE html>
<html>
	<head>
		<meta charset="utf-8">
		<meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="styles.css" type="text/css" />
    <script type="text/javascript" src="jquery-2.1.3.min.js"></script>
    <script type="text/javascript" src="script.js"></script>
    <link rel="icon" href="favicon.ico" type="image/x-icon" />
    <script>
      window.onload = setInterval(function() {
        reloadData();
      }, 2000);
    </script>
  </head>            
	<body>
    <table>
      <tr>
        <td><img src="logo.png"></td>
      </tr>
      <tr>
        <td class="freq freq_l"><h1 id="freq"><?frequency(0)?></h1></td>
        <td class="freq freq_r"><h4><?frequency(1)?></h4></td>
        <td><button onclick="get('frequency=1')" style="margin-bottom: 1px;">UP</button><br />
            <button onclick="get('frequency=-1')" style="margin-top: 1px;">DN</button></td>
      </tr>
			<tr>
				<td><div class="vol" id="vol"></div></td>
				<td>VOLUME</td>
			</tr>
    </table>
	</body>
</html>