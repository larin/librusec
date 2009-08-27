<?php
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function libDonateAdd($cod, $sum, $pt) {
  static $paitype;
  if (!$pt) $pt = $paytype;
  else $paytype = $pt;
  preg_match('/u(\d+)t(\d)a(\d+)/',$cod,$m);
  if (!$m[3]) preg_match('/u(\d+)t(\d)/',$cod,$m);
  $sum = (integer)($sum*100);
  $pt = strtolower($pt);
  if ($sum > 0 && $m[2] > 0) {
    Insert(libdonations, 'uid,Sum,Type,AvtorId,PayType',"$m[1], $sum, $m[2], '$m[3]','$pt'");
    if ($sum > 999) Update(users, 'noadv=1', "uid = $m[1]");
  } else {
    print "libDonateAdd($cod, $sum, $pt)<br>";
    print "  if ($sum > 0 && $m[2] > 0) ";
    exit;
  }
}

function LibDonate() {
  if ($_GET['stat']=='paid') lgo ("donate/stat/paid");
  drupal_add_js('modules/librusec/md5.js');
  Global $user;
  $u = 'u'.$user->uid;
  if (SuperUser()) {//$debug=1;
    if (arg(1) == 'block') {
      $a = arg(2);
      Update (libavtorname, 'NoDonate=1',"AvtorId=$a");
      lgo("/a/$a");
    }  
    if ($_GET[sum] && $_GET[paytype]) {
      $cod = $_GET[code];
      if (!$cod) $cod = 'u0t4a';
      libDonateAdd($cod, $_GET[sum], $_GET[paytype]);
      lgo("/donate/stat/paid");
    }
    if ($cod = $_POST['codes']) {
      foreach (split("\n", $cod) as $l) {
         $q = preg_split('/\s+/', $l);
         for ($i=0;$i<3;$i++) $q[$i] = trim($q[$i]);
         libDonateAdd($q[1], $q[0], $q[2]);
      }
      lgo("/donate/stat/paid");
    }
    if (arg(1) == 'paid' && ($a = arg(2)) && ($s = arg(3))) {
      $sum = (integer)($s*100);
      Insert (libdonations, 'uid,Sum,Type,AvtorId',"1, $sum, 9, $a");
      lgo("/donate/stat/4avt");
    }
    $paid = "<tr><td>9.<input type=radio name=tp value=9 onChange=FF()>Переведено <input type=button onClick=SavePaid() value='Переведено по назначению'></tr>";
  }  

  if (arg(1) == 'stat') {
    if (!($user->uid > 0 && (Sel("uid FROM libdonations WHERE uid = $user->uid") || Sel("uid FROM libavtorname WHERE uid = $user->uid")))) 
      return "Статистика видна только участникам - тем, кто платил, и авторам.
<br>Новые платежи обрабатываются с задержкой до одного дня по техническим причинам.
<br>Если Вы автор - сообщите мне свой логин и я открою для Вас тоже. 
<br>Если нет - переведите хотя бы один цент.";

    switch (arg(2)) { 
      case '4avt':
        return libdonate4avt();
      case avtors: 
        return libdonateparts();
      case paid:
        return libdonatepaid();
      case table:
        return libdonatetable();  
      case mes:
        return libdonatepaid(arg(3));
      case user:
        return libdonateuser(arg(3));
      default:
        return "Непонятная метка `$act`"  ;
     }   
  }
  
  if (arg(1) == 'SMS') {      
    return libDonateSMS();
  }
  if (arg(1) == 'sms') {      
    return libDonateSMS1();
  }
//Формочка сбора денег

  if (arg(1) == 'author' && ($a = arg(2))) $checked = 'checked';
  $crc = md5("librusec:20:0:1a1a1a1a:shp_item=2"); 
  $t = $a ? 1 : 4;
  $sms1 = $user->uid*1000000+$a+$t*1000000000000;
  $desc = $u."t$t"."a$a";
  $crc1 = md5("4281::$sms1::20::1::$desc::1");
  return "<script><!--\nvar u='$u'\nvar uid=$user->uid\n".
'
var f,a,la,d,aid,s,t=4,w;
var q = new Array();
function FF(p) {
 f=document.forms; var f0=f["donate"];
 if (p>0) f0.tp[0].checked=1;
 nm=document.getElementById("aname");
//считываем значения из нулевой формы
 for (i=0;f0.tp[i];i++) if (f0.tp[i].checked) t=i+1;
 s=f0.summa.value;
 a=f0.av.value;
 if (a!=la) {
  if (la=a) 
   if ($.get) $.get("/AJAX.php?op=aname&s="+encodeURI(a), function(m){q=m.split(":");aid=q[0];FF();nm.innerHTML="<a href=/a/"+q[0]+">"+q[1]+"</a>"}) 
   else aid=a;
  else aid=q[1]=nm.innerHTML=""
 }
//распихиваем их по прочим формам
 for (i=1;d=f[i];i++) {   
  d=d.elements;
  for (k=0;d[k];k++) {
   id=d[k].id;
   desc=u+"t"+t+"a"+aid;
   smsid=uid*1000000+1*aid+t*1000000000000;
   if (id=="smsid") d[k].value=smsid;
   if (id=="sum") d[k].value=s;
   if (id=="desc") d[k].value=desc;
   if (id=="crc") d[k].value=hex_md5("librusec:"+s+":0:1a1a1a1a:shp_item=2");
   if (id=="crc1") d[k].value=hex_md5("4281::"+smsid+"::"+s+"::1::"+desc+"::1");
   if (id=="sms") d[k].value="hmlib "+desc;
  } 
 }     
}
function SavePaid() {
 f=document.forms; var f0=f["donate"];
 s=f0.summa.value;
 a=f0.av.value;
 location.href= "/donate/paid/"+aid+"/"+s;
}
--></script>' . "<h4>Оплата</h4>
<noscript><p>Убедительная просьба включить JavaScript для корректной работы формы</p></noscript>
<form name=donate>
<br>Сумма платежа <input size=2 id=summa value=20 onChange=FF()>USD
<br>Вы можете распределить свои деньги следующими способами:
<table border=0>
<tr><td>1.<input type=radio name=tp value=1 onChange=FF() $checked>Указанному автору (впишите ID или фамилию и нажмите TAB) 
          <input id=av onBlur=FF(1) size=8 value='$a'> &nbsp; <span id=aname></span>
<tr><td>2.<input type=radio name=tp value=2 onChange=FF()>Поделить между прочитанными авторами (согласно статистике на <a href=/polka>полке</a> на момент дележа) (только для зарегистрованных пользователей)</tr>
<tr><td>3.<input type=radio name=tp value=3 onChange=FF()>Поделить между всеми авторами (согласно <a href=/donate/stat/avtors>общей статистике библиотеки</a>)</tr>
<tr><td>4.<input type=radio name=tp value=4 onChange=FF()>Передать на нужды Либрусека</tr>
$paid
</table>
</form>
<br>Суммы, предназначенные доступным российским авторам, будут им переданы в удобном для них виде.
<br>Суммы, предназначенные оставившим нас авторам, будут использованы на нужды библиотеки.
<br>Издатели, наследники и прочие правообладатели не учитываются.
<br>Статистика пока собирается вручную, поэтому может быть задержана на пару дней.
<br><br>
<b>Принимаются:</b>

<table border=3 bordercolor=gray><tr>
<th>MoneyBookers
<th>PayPal
<th>WU
<th>Webmoney
<th>SMS
<th>Прочие
<tr>
<td>(кредитки, переводы)
<td>(кредитки)
<td>(наличные)
<td>
"//<td>(50 стран)
."
<td>(РФ, Казахстан, Украина) 
<td>(обменник)
<tr>
<td><form action=https://www.moneybookers.com/app/payment.pl method=post target=_blank>
<br>
<input type=hidden name=pay_to_email value='mb@rus.ec'>
<input type=hidden name=status_url value='lib@rus.ec'>
<input type=hidden name=language value='РУС'>
<input type=hidden name=amount value=10 id=sum>
<input type=hidden name=currency value=USD>
<input type=hidden name=detail1_description id=desc value=$desc>
<input type=hidden name=detail1_text value='Пожертвование'>
<input type=submit value='Заплатить'>
</form>
<td><form action=https://www.paypal.com/cgi-bin/webscr method=post>
<br>
<input type=hidden name=cmd value=_xclick>
<input type=hidden name=business value='larin@linuxcentro.com'>
<input type=hidden name=item_name value=Librusec>
<input type=hidden name=item_number id=desc value=$desc>
<input type=hidden name=no_shipping value=1>
<input type=hidden name=return value=http://lib.rus.ec/>
<input type=hidden name=cancel_return value=http://lib.rus.ec/>
<input type=hidden name=currency_code value=USD>
<input type=hidden name=bn value='PP-DonationsBF'>
<input type=hidden name=amount value='10.00' id=sum>
<input type=submit name=submit value='Заплатить'>
</form>
<td valign=top><a href=/contact>Ilya Larin, Ecuador</a>
<td valign=top>
Z630179967226<br>R377382658187 
<form action=https://merchant.webmoney.ru/lmi/payment.asp method=POST>
 <input type=hidden name=LMI_PAYMENT_AMOUNT value=10 id=sum>
 <input type=hidden name=LMI_PAYMENT_DESC value=$desc id=desc>
 <input type=hidden name=LMI_PAYEE_PURSE value=Z630179967226>
 <input type=submit value='Заплатить'>
</form>
"
/*
<td valign=top>
<form action=http://2a95.bank.smscoin.com/bank/ method=POST>
<input name=s_purse type=hidden value=4281 />
<input name=s_order_id type=hidden id=smsid value=$sms1/>
<input name=s_amount type=hidden value='10.0' id=sum />
<input name=s_clear_amount type=hidden value=1 />
<input name=s_description type=hidden id=desc value=$desc />
<input name=s_sign type=hidden id=crc1 value='$crc1' />
<input type=submit value='Послать' />
</form>
*/
."
<td valign=top>
<form>Пришлите слово <input readonly id=sms value=hmlib><br>на номер 4161($4,75).</form>
<td>
<form action=https://www.roboxchange.com/ssl/calc.asp method=POST>
<input type=hidden name=mrh value=librusec>
<input type=hidden name=out_summ value=10 id=sum>
<input type=hidden name=inv_id value=0>
<input type=hidden name=inv_desc value=$desc id=desc>
<input type=hidden name=crc value=$crc id=crc>
<input type=hidden name=shp_item value=2>
<input type=submit value='Заплатить'>
</form>

<tr><td colspan=3 align=center>
<form>Просьба при переводе указывать код: <input readonly id=desc></form>
<td colspan=4>
</table>
<br>
<h4>Подписка</h4>
<p>(впишите сумму в графе Сумма платежа, и она будет сниматься ежемесячно)</p>

<form action=https://www.paypal.com/cgi-bin/webscr method=post>
<input type=image src=https://www.paypal.com/en_US/i/btn/x-click-but20.gif border=0 name=submit>
<img border=0 src=https://www.paypal.com/en_US/i/scr/pixel.gif width=1 height=1>
<input type=hidden name=cmd value='_xclick-subscriptions'>
<input type=hidden name=business value='larin@linuxcentro.com'>
<input type=hidden name=item_name value=Librusec>
<input type=hidden name=item_number id=desc value=$desc>
<input type=hidden name=no_shipping value=1>
<input type=hidden name=no_note value=1>
<input type=hidden name=currency_code value=USD>
<input type=hidden name=lc value=RU>
<input type=hidden name=bn value='PP-SubscriptionsBF'>
<input type=hidden name=a3 value='10.00' id=sum> 
<input type=hidden name=p3 value=1>
<input type=hidden name=t3 value=M>
<input type=hidden name=src value=1>
<input type=hidden name=sra value=1>
</form>
<br><br>
<br><b><a href=/donate/stat/paid>Полученные платежи</a></b>
<br><b><a href=/donate/stat/table>Сводная таблица</a></b>
<br><b><a href=/donate/stat/4avt>Распределение по авторам</a></b>
<script><!-- 
document.onload=FF(0) 
--></script>";

//http://lib.rus.ec/donate/wm/OK?LMI_PAYMENT_NO=0&LMI_SYS_INVS_NO=228&LMI_SYS_TRANS_NO=489&LMI_SYS_TRANS_DATE=20071129+02%3A47%3A17&=
//Webmoney Z630179967226, R377382658187<br>

}

function libdonatetable($mes = '') {
  Global $user;
  set_title('Полученные платежи');
  $sth = SELECT ("SUM(Sum) s, substr(time, 1, 7) t, type FROM `libdonations` GROUP BY 2,3 ORDER BY 2 DESC, 3");
  $r = "<table><tr><th>Месяц<th>Автору<th>Авторам<th>Библиотеке<th>Всего\n";
  while ($a1 = dbf($sth)) {
    if ($last != $a1->t) {
      if ($last) $r .= "<tr><td><a href=/donate/stat/mes/$last>$last</a><td>$s[1]<td>$s[3]<td>$s[4]<td>$$s[5]";
      $last = $a1->t;
      $s = array();
    }
    $v = (int)($a1->s/100 + .49);
    $s[$a1->type] = '$'.$v;
    $s[5] += $v;
  }
  return $r."<tr><td><a href=/donate/stat/mes/$last>$last</a><td>$s[1]<td>$s[3]<td>$s[4]<td>$$s[5]</table>";
}

function libdonate4avt() {
  Global $user;
  set_title('Полученные для авторов платежи');
  $sth = SELECT ("AvtorId, SUM(Sum) AS S, SUM(1) AS N FROM libdonations WHERE Type = 1 GROUP BY 1");
  while ($a1 = dbf($sth)) {
    $SUM1[$a1->AvtorId] += $a1->S; // получено лично для автора
    $SUMN[$a1->AvtorId] += $a1->N; // раз
  }  
  $sth = SELECT ("AvtorId, SUM(Sum) AS S, SUM(1) AS N FROM libdonations WHERE Type = 99 GROUP BY 1");
  while ($a1 = dbf($sth)) 
    $SUM2[$a1->AvtorId] += $a1->S; // уже отдано автору

// разделить между всеми
  $tosplit = Sel ("SUM(Sum) FROM libdonations WHERE Type = 3"); // к разделу
  if (!$tosplit) return $r;

  $sth = SELECT ("SUM(IF(FileType = 'djvu' OR FileType = 'pdf',FileSize/7,FileSize)*N) AS S, AvtorId FROM liblog JOIN libbook USING(BookId) JOIN libavtor USING (BookId) JOIN libavtorname USING (AvtorId) WHERE NOT NoDonate GROUP BY AvtorId ORDER BY S DESC LIMIT 100");
  while ($a1 = dbf($sth)) {
    $tot += $a1->S;
    $SUM3[$a1->AvtorId] += $a1->S;
  } 
  $r = "<ol>";
  arsort($SUM1);
  foreach ($SUM1 as $a => $s) {
    $r .= "<li>".avl($a).sprintf(' получил $%01.2f ', $s/100);
    if ($SUMN[$a] > 1) $r .= " от ".$SUMN[$a]." читателей";
    if ($s1 = (int) (($SUM3[$a]/$tot)*$tosplit))
      $r .= sprintf(' и $%01.2f при разделе, итого $%01.2f', $s1/100, ($s+$s1)/100);
    if ($s3 = Sel("SUM(Sum) FROM libdonations WHERE AvtorId = $a AND Type = 9"))
      $r .= sprintf(". Из этого выплачено $%01.2f, осталось $%01.2f", $s3/100, ($s+$s1-$s3)/100);
    if ($s+$s1-$s3 >= 5000) $r .=  sprintf(" &nbsp; &nbsp; &nbsp; <b>Пора переводить: $%01.2f</b>", ($s+$s1-$s3)/100);
  }
  $r .= "</ol>";
  return $r;
}

function libdonateparts() {
  Global $user;
  set_title("Доли авторов");
  $tosplit = Sel ("SUM(Sum) FROM libdonations WHERE Type = 3");
  $r .= "Доли авторов распределяются пропорционально выкачанным килобайтам. Размер djvu и pdf делится на 7, остального берётся как есть, fb2 непакованный.<br>По умолчанию участвует сто самых популярных, иностранные, не желающие и покинувшие нас вычёркиваются вручную. Просьба сообшать о таковых<br>Если будет смысл, то количество участвующих авторов будет увеличено.<br>На данный момент к разделу \$".($tosplit/100);

  $limit = $user->name == larin ? 200 : 100;
  $sth = SELECT ("SUM(IF(FileType = 'djvu' OR FileType = 'pdf',FileSize/7,FileSize)*N) AS S, AvtorId FROM liblog JOIN libbook USING(BookId) JOIN libavtor USING (BookId) JOIN libavtorname USING (AvtorId) WHERE NOT NoDonate GROUP BY AvtorId ORDER BY S DESC LIMIT $limit");
  $r .= "<ol>";

  for ($i=0;$a1=dbf($sth);$i++) {  
    $tot += $a1->S;
    $AV[$i] = $a1->AvtorId;
    $S[$i] = $a1->S;
  }

  for ($i=0;$S[$i];$i++) {
    $p = (int)(10000*$S[$i]/$tot);
    $p /= 100;
    if (strlen($p)<4) {if ($p == 1)$p='1.00';else $p.='0';}
      $dolya = (int)($tosplit*$S[$i]/$tot);
      $dolya /= 100;
      if (strlen($dolya)<4){if ($dolya==(int)$dolya) $dolya.='.00';else $dolya.='0';}
      $r .= "<li>$p% (\$$dolya) ".avl($AV[$i]);
      if (SuperUser()) $r .= " <a href=/donate/block/$AV[$i]>Снять с соревнования</a>";
    }
  $r .=  "</ol>";
  return $r;
}    
    
function libdonatepaid($mes = '') {
  if (SuperUser()) {
    $r = "Добавить в статистику Код - Сумма - Система<form action=/donate><input name=code> <input name=sum value=10> <input name=paytype value=paypal> <input type=submit></form>";
    $r .= "<form action=/donate method=POST><p>Платёж на строчку, Сумма Код Система, через пробел</p><textarea rows=5 cols=66 name=codes></textarea><input type=submit></form>";
  }
  
  if ($mes) {
    $mes = addslashes($mes);
    $w = "WHERE Type < 5 AND substr(time, 1, 7) = '$mes'"; 
    set_title("Платежи, полученные за $mes");
  } else {
    set_title("Полученные платежи");
    $w = "WHERE Type < 5"; 
  }
  $sth = pgq ("SELECT *, DATE_FORMAT(Time,'%e %c') as dt FROM libdonations $w ORDER BY 1 DESC", Sel ("COUNT(*) FROM libdonations $w"));
  $r .= "<table><tr><th>Дата<th>Участник<th>Сумма<th>Кому<th>Платёжная система";
  while ($a1=dbf($sth)) {
    switch ($a1->Type) {
      case 1:$rasp=avl($a1->AvtorId); break;
      case 2:$rasp="<a href=/polka/show/$a1->uid>распределить между прочитанными</a>";break;
      case 3:$rasp="<a href=/donate/stat/avtors>распределить между всеми</a>";break;
      case 4:$rasp="на Либрусек";break;
      default: $rasp='непонятно';break;
    }
    $sum = $a1->Sum/100;
    if ($u = $a1->uid) 
      $username = "<a href=/user/$u>".Sel("name FROM users WHERE uid = '$u'")."</a>";
    else 
      $username = '';  
    $r .= "<tr><td align=right>$a1->dt &nbsp; <td>$username<td>\$$sum<td>$rasp<td>$a1->PayType";
  }
  $sum = sprintf("%01.2f", Sel("SUM(Sum) FROM libdonations $w")/100);
  $t1 = (int)(Sel("UNIX_TIMESTAMP(Max(Time)) - UNIX_TIMESTAMP(Min(Time)) FROM libdonations $w")/3600/24);
  $r .= "<tr><td><tr><td><td><b>Итого</b><td>\$$sum<td>за $t1 дней";
  if ($t1) {$t2 = (int)($sum/$t1 + .5); $r .=", в среднем \$$t2 в день";}
  $r .= "</table>";
  return $r.theme('pager');
}
    
function libDonateSMS1() { 
  switch(arg(2)) {
    case FAIL:
      return "Попытка SMS-платежа не удалась. <a href=/donate>Попробуйте другие способы</a>";
    case OK: 
       $sum = $_POST[s_amount];
       $smsid = $_POST[s_order_id];
       $t = (int)($smsid/1000000000000);
       $smsid %= 1000000000000;
       $u = (int)($smsid/1000000);
       $smsid %= 1000000;
       $a = $smsid;
       libDonateAdd("u$u"."t$t"."a$a", $sum, "sms"); 
    return "Принят SMS-платеж.".($u > 0 ? "<a href=/donate/stat/paid>Статистика</a>" : '');
  }
}   
    
function libDonateSMS() {
  $SecretKey="seccluch";
  $DebugSecretKey="test";

// If parameter 'test' exists then send it's value or 'OK' if empty value. This code is necessary for checking accessibility of script.
  if(isset($_REQUEST['test'])) {
	$smsResponse="OK";
	if(trim($_REQUEST['test'])!="")
		$smsResponse=$_REQUEST['test'];
  } else {
	if (isset($_REQUEST['smsText'])) $smsRequestEncoded = $_REQUEST['smsText'];
	if (isset($_REQUEST['phone'])) $phone = $_REQUEST['phone'];
	if (isset($_REQUEST['abonentId'])) $abonentId = $_REQUEST['abonentId'];
	if (isset($_REQUEST['country'])) $country = $_REQUEST['country'];
	if (isset($_REQUEST['serviceNumber'])) $serviceNumber = $_REQUEST['serviceNumber'];
	if (isset($_REQUEST['operator'])) $operator = $_REQUEST['operator'];
	if (isset($_REQUEST['now'])) $now = $_REQUEST['now'];
	if (isset($_REQUEST['md5key'])) $md5key = $_REQUEST['md5key'];
	if (isset($_REQUEST['retry'])) $retry=$_REQUEST['retry'];
	
	if (isset($smsRequestEncoded) && isset($phone) && isset($abonentId) && isset($country) && isset($serviceNumber) && isset($now) && isset($md5key)) {
		// Place code here if you want log request into your database
		
		// Request validation
		$md5CheckSrc=$serviceNumber.$smsRequestEncoded.$country.$abonentId.$SecretKey.$now;
		if(isset($retry))
			$md5CheckSrc.=$retry;
		if(isset($_REQUEST['debug'])){
			$md5CheckSrc.=$_REQUEST['debug'];
			$md5CheckSrc.=$DebugSecretKey;
		}
		$md5Check=md5($md5CheckSrc);

		// Validate md5key
		if(strcasecmp($md5Check,$md5key) == 0) {
			// Get decoded sms message
			$smsRequest=base64_decode($smsRequestEncoded);
		
			// Place your code here to construct SMS response
			// For example:
			$smsResponse="SMS request received: ".$smsRequest;
		} else {
			// Validation failed. Construct security error to i-Free Service 
			$errorText="Security error: md5key check failed";
		}
     } else {
		// Construct general error to i-Free Service
		$errorText="General error: not all variables set";
     }
  }

  // Construct response
  $httpResponse="<Response>";

  // Add sms message to response
  if(isset($smsResponse)) $httpResponse.="<SmsText><![CDATA[".$smsResponse."]]></SmsText>";
  // Add error message to response
    if(isset($errorText)) $httpResponse.="<ErrorText><![CDATA[".$errorText."]]></ErrorText>";
  $httpResponse.="</Response>";
  print $httpResponse;
  exit;
}

function libdonateuser($uid) {
  Global $user; 
  if ($uid == 'me') $uid = $user->uid;
  $name = $uid ? Sel('name FROM users WHERE uid = %d', $uid) : 'Аноним';
  set_title("Платежи, полученные от пользователя $name");
  $sth = SELECT("*, DATE_FORMAT(Time,'%e %c') as dt FROM libdonations WHERE uid = %d ORDER BY 1 DESC", $uid);
  $r .= "<table><tr><th>Дата<th>Сумма<th>Кому<th>Платёжная система";
  while ($a1=dbf($sth)) {
    switch ($a1->Type) {
      case 1:$rasp=avl($a1->AvtorId); break;
      case 2:$rasp="<a href=/polka/show/$a1->uid>распределить между прочитанными</a>";break;
      case 3:$rasp="<a href=/donate/stat/avtors>распределить между всеми</a>";break;
      case 4:$rasp="на Либрусек";break;
      default: $rasp='непонятно';break;
    }
    $sum = $a1->Sum/100;
    $r .= "<tr><td align=right>$a1->dt &nbsp; <td>\$$sum<td>$rasp<td>$a1->PayType";
  }
  return $r."</table>";
}