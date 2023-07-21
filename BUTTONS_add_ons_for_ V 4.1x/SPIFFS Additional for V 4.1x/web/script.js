function get(n) {
  const xhr = new XMLHttpRequest();
  xhr.open("GET", "command.php?" + n);
  xhr.send();
}

function reloadData() {
	$.ajax({
    type: "POST",
    url: "get.php",
    success: function(answ) {
			let arrAnsw = answ.split('\r\n');
			document.getElementById("freq").innerHTML = arrAnsw[0];
			document.getElementById("vol").style.width = String(arrAnsw[1] * 2) + "px";
		}
  });
}
