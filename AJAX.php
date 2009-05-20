<?php
error_reporting(E_ERROR | E_WARNING | E_PARSE);
$t1 = microtime(TRUE);
Global $DBH, $u, $m;
require_once ('./sites/default/settings.php'); //тут лежат логины к мускулу
function variable_get($name, $default) {  global $conf;  return isset($conf[$name]) ? $conf[$name] : $default;}
$memcachemodule = 'modules/memcache/dmemcache.inc';
if (file_exists($memcachemodule)) require_once ($memcachemodule);
else $memcachemodule = '';
preg_match('|://(.+)@(.+)/(.+)$|',$db_url, $m);  //разбираем в поисках логина к базе.
$up = split(':', $m[1]);  //user:pwd
$DBH = mysql_connect($m[2], $up[0], $up[1]) or die("Could not connect: " . mysql_error()); 
mysql_select_db($m[3],  $DBH) or die(mysql_error());
mysql_query("SET CHARSET utf8");
mysql_query("SET NAMES utf8");
function Insert($tbl, $col, $val) { return mysql_query("INSERT INTO $tbl ($col) values ($val)");}
function SELECT($st) { return mysql_query("SELECT $st");}
function Sel($st) { return ($sth = SELECT($st)) && mysql_num_rows($sth) > 0 ? mysql_result($sth, 0) : ''; }
function Update ($table, $columns, $cond) { return mysql_query("UPDATE $table SET $columns WHERE $cond");}
function dbf ($s)     {return mysql_fetch_object($s);}
function S($st)       {return dbf(SELECT($st));}
function SQLRows($st) {return mysql_num_rows(SELECT($st));}
function p($t) { return addslashes(trim(urldecode($_GET[$t])));}

function AvtorName($a, $t=' ') {
  $an = Sel ("CONCAT_WS('$t', FirstName, MiddleName, LastName) from libavtorname WHERE AvtorId = $a");
  $nick = Sel ("NickName from libavtorname WHERE AvtorId = $a");
  if ($nick) $an .= " ($nick)";
  $an = preg_replace('/>/','&gt;',$an);
  $an = preg_replace('/</','&lt;',$an);
  return $an;
}

function avl($a, $t=' ', $o='') {  
  $cnt = '';
  if ($o === cnt) $cnt = '('.Sel("count(*) FROM libbook JOIN libavtor USING(BookId) WHERE AvtorId = $a AND NOT (Deleted&1)").')'; 
  return "<a href=/av?$a>".AvtorName($a, $t)."</a>$cnt"; 
}

$u = 0;
foreach ($_COOKIE as $cook) if ($u = Sel("uid FROM sessions WHERE sid = '$cook'")) break;

if ($u) 
  if (Sel( "uid FROM users_roles WHERE uid=$u AND rid=4")) 
    exit;

$uu = $u ? $u : $_SERVER['REMOTE_ADDR'];

if (isset($_GET['b'])) $b = (integer)$_GET['b'];
if (isset($_GET['a'])) $a = (integer)$_GET['a'];
if (isset($_GET['op'])) $op = $_GET['op'];
elseif (isset($_POST['op'])) $op = $_POST['op'];
if (!$op) exit;

switch ($op) {
  case 'ss':
    if ($b && $uu) 
      db_query("INSERT INTO libreaded (BookId, UserId) VALUES($b, $uu) ON DUPLICATE KEY UPDATE BookId = BookId");    
  exit;

  case 'polka':
    if (!$u) exit;
    $b = (integer)$_POST['BookId'];
    $b = Sel ("BookId FROM libbook WHERE BookId = $b");
    $txt = addslashes($_POST['text']);
    $f = $_POST['Flag'] == 'true' ? 'h' : '';
    if ($b > 0 && $u && $txt != 'undefined') 
      mysql_query("INSERT INTO libpolka (BookId, UserId, Text, Flag) VALUES($b, $u, '$txt', '$f') ON DUPLICATE KEY UPDATE Text = '$txt', Flag='$f'");
 exit;

  case 'aname':
    $a = addslashes($_GET['s']);
    $a = Sel ("AvtorId FROM libavtorname WHERE AvtorId='$a' OR LastName='$a' OR '$a'=CONCAT(FirstName,' ',LastName) LIMIT 1"); 
    if ($a) print "$a:".AvtorName($a);
 exit;

  case 'setrate':
    $r = 1*($_GET['r']);
    if (!$b || !$uu || !($r>0)) exit;
    mysql_query("INSERT INTO librate (BookId, UserId, Rate) VALUES($b, $u, $r) ON DUPLICATE KEY UPDATE Rate=$r");
    $cid = "librate$uu";
    if ($memcachemodule) dmemcache_delete($cid, librusec);
    else mysql_query ("DELETE FROM librusec WHERE cid = '$cid'");
 exit;

  case 'setquality':
    $q = 1*($_GET['q']);
    if (!$b || !$u || !$q) exit;
    mysql_query("INSERT INTO libquality (q, BookId, uid) VALUES ($q, $b, $u) ON DUPLICATE KEY UPDATE q=$q");
    $cid = 'libquality'.$b;
    if ($memcachemodule) dmemcache_delete($cid, librusec);
    else mysql_query ("DELETE FROM librusec WHERE cid = '$cid'");
 exit;

  case 'setuseropt':
    $o = addslashes($_GET['o']);
    $v = addslashes($_GET['v']);
    if (!$o || !$v || !$u) exit;
    mysql_query("INSERT INTO libuseropt (User, Opt, Value) VALUES($u, '$o', '$v') ON DUPLICATE KEY UPDATE Value = '$v'");
  exit;
 
  case 'getuseropt':
    $o = addslashes($_GET['o']);
    if (!$o || !$u) exit;
    print Sel ("Value FROM libuseropt WHERE User = $u AND Opt = '$o'");
  exit;

  case 'setlang':  
   $l = addslashes($_GET['l']);
   if (!$b || !$l || !$u) exit;
   $l1 = Sel("Lang FROM libbook WHERE BookId = $b");
   if ($l == $l1) exit;
   Update ('libbook', "Lang = '$l'", "BookId = $b");
   $un = Sel("name FROM users WHERE uid=$u");
   Insert ('libactions', 'UserName, ActionSQL, ActionDesc, ActionUndo', "\"$un\", \"UPDATE libbook SET Lang = '$l' WHERE BookId = $b\", 
           \"Set lang $l for $b\", \"UPDATE libbook SET Lang = '$l1' WHERE BookId = $b\"");
  exit;

  case 'setyear':  
   $l = (integer) $_GET['l'];
   if (!$b || !$l || !$u) exit;
   Update ('libbook', "Year = '$l'", "BookId = $b");
   $un = Sel("name FROM users WHERE uid=$u");
   Insert ('libactions', 'UserName, ActionSQL, ActionDesc, ActionUndo', "\"$un\", \"UPDATE libbook SET Year = '$l' WHERE BookId = $b\", 
           \"Set Year $l for $b\", \"UPDATE libbook SET Year = '$l1' WHERE BookId = $b\"");

  case 'setyear':  
   $l = (integer) $_GET['l'];
   if (!$b || !$l || !$u) exit;
   Update ('libbook', "Year = '$l'", "BookId = $b");
   $un = Sel("name FROM users WHERE uid=$u");
   Insert ('libactions', 'UserName, ActionSQL, ActionDesc, ActionUndo', "\"$un\", \"UPDATE libbook SET Year = '$l' WHERE BookId = $b\", 
           \"Set Year $l for $b\", \"UPDATE libbook SET Year = '$l1' WHERE BookId = $b\"");
  exit;  
  
  case 'setuid':  
   if (!$a || !$u) exit;
   $l = addslashes($_GET['l']);
   $uid = 0;
   if ($l == '' OR ($uid = Sel ("uid FROM users WHERE name = '$l'"))) {
     $ouid = Sel ("uid FROM libavtorname WHERE AvtorId = $a");
     Update ('libavtorname', "uid = $uid", "AvtorId = $a");
     $un = Sel("name FROM users WHERE uid=$u");
     Insert ('libactions', 'UserName, ActionSQL, ActionDesc, ActionUndo', "\"$un\", 'UPDATE libavtorname SET uid = $uid WHERE AvtorId = $a', 
           'Set Avtor UID to $uid for $a', 'UPDATE libavtorname SET uid = '$ouid' WHERE AvtorId = $a'");
   }           
   print "uid = $uid";
 exit;
}



