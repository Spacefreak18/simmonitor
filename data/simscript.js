
function updateFlag(flagstatus) {
  switch(flagstatus) {
    // blue
    case 4:
      document.body.style.backgroundImage=""
      document.body.style.backgroundColor = "#0000ff";
      break;
    // yellow
    case 1:
      document.body.style.backgroundImage=""
      document.body.style.backgroundColor = "#ffff00";
      break;
    // white
    case 5:
      document.body.style.backgroundImage=""
      document.body.style.backgroundColor = "#000000";
      break;
    // black
    case 6:
      document.body.style.backgroundImage=""
      document.body.style.backgroundColor = "#ffffff";
      break;
    // chequered
    case 3:
      document.body.style.backgroundImage="url(https://wallpapercave.com/wp/wp4980392.png)"
      break;
    // black and orange
    case 8:
      document.body.style.backgroundImage="url(https://xmple.com/wallpaper/squares-orange-checkered-black-1920x1080-c2-ff8c00-000000-l-120-a-85-f-2.svg)"
      break;
    // orange
    case 9:
      document.body.style.backgroundImage=""
      document.body.style.backgroundColor = "#ffa500";
      break;
    default:
      document.body.style.backgroundImage=""
      document.body.style.backgroundColor = "#3cd070";
  }
}


function simScript() {

  flag = parseInt(document.getElementById("flag").textContent);
  updateFlag(flag);
}


