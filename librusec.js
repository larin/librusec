function UCC(xxx) {
  for (i = 1; i < document.forms["bk"].length; i++) {
    var q = document.forms["bk"].elements[i];
    if (q.id != "sa") q.checked = xxx;
  }
}

function UCCg(xxx, g) {
  for (i = 1; i < document.forms["bk"].length; i++) {
    var q = document.forms["bk"].elements[i];
    if (q.id.split('-')[0] == g) q.checked = xxx;
  }
}

function UCCs(xxx, g) {
  for (i = 1; i < document.forms["bk"].length; i++) {
    var q = document.forms["bk"].elements[i];
    if (q.id.split('-')[1] == g) q.checked = xxx;
  }
}

function confirmmassdownload() {
  n = 0;
  f = document.forms["bk"];
  for (i = 1; i < f.length; i++)    
      if (f.elements[i].checked && f.elements[i].name > 0)  n++  
  if (n < 1) return false;
  return confirm("Уверены в необходимости выкачать "+n+" книг одним файлом?");
}

function confirmmassdelete() {
  n = 0;
  f = document.forms["bk"];
  for (i = 1; i < f.length; i++)    
      if (f.elements[i].checked && f.elements[i].name > 0)  n++  
  if (n < 1) return false;
  return confirm("Уверены в необходимости удаления "+n+" книг?");
}

function confirmmassundelete() {
  n = 0;
  f = document.forms["bk"];
  for (i = 1; i < f.length; i++)    
      if (f.elements[i].checked && f.elements[i].name > 0)  n++  
  if (n < 1) return false;
  return confirm("Уверены в необходимости восстаноления "+n+" книг?");
}

function clearchbox() {
  f = document.forms["bk"];
  for (i = 0; i < f.length; i++) 
    if (f.elements[i].name > 0)
      f.elements[i].checked = ""
}

function cnf(ask, todo) {
  if(confirm(ask)) location.href = todo;
}

var ltm = new Array();
var ltxt = new Array();
var ltxt1 = new Array();
var ii=1;

function polkasave(b) {
  if (!(b>0)) return;
  var txt = $("#"+b).val()
  var f = $("#h"+b);
  var ch = f.attr("checked");
  if (ltxt[b] == -1) ltxt[b]=txt+ch;
  if (ltxt[b] == txt+ch) return;  
  ltxt[b]=txt+ch

//  var tm = new Date; tm = tm.getTime()
//  if (tm - ltm[b] < 1111) { setTimeout(polkasave, 444); return; } 
//  ltm[b] = tm

  jQuery.post("/AJAX.php",  { op:'polka', BookId:b, text:txt, Flag:ch}) 
}  

function setrate(b) {
 r = document.getElementById('rate'+b).value;
 jQuery.get('/AJAX.php', {op:'setrate',b:b,r:r}); 
}

function setquality(b) {
 q = document.getElementById('q'+b).value;
 jQuery.get('/AJAX.php', {op:'setquality',b:b,q:q}); 
}
 
function setuseropt(o) {
 v = document.getElementById('useropt').value;
 jQuery.get('/AJAX.php', {op:'setuseropt',o:o,v:v}); 
}

jQuery(document).ready(function (){$("textarea.polka").each(function(){b=this.id; ltxt[b]=-1; polkasave (b)})})

function setlang(b){
 l = document.getElementById('lang'+b).value;
 jQuery.get('/AJAX.php', {op:'setlang',b:b,l:l}); 
}

function setyear(b){
 l = document.getElementById('year'+b).value;
 jQuery.get('/AJAX.php', {op:'setyear',b:b,l:l}); 
}

function setuid(a){
 l = document.getElementById('uid'+a).value;
 jQuery.get('/AJAX.php', {op:'setuid',a:a,l:l}); 
}

function show(id) {
 el = document.getElementById(id).style;
 if( el.position != 'absolute' ) {
   el.position = 'absolute';
   el.left = '-4000px';
 } else {
   el.position = 'relative';
   el.left = '0px';
 }
}