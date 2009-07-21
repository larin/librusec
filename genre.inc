<?php
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function libGenre() {
  Global $user, $ShowEN;  
  if (!$ShowEN) $asql = "AND Lang='ru'";
  $g = arg(1);
  if (!$g || $g == 'all' || $g == 'ge') { 
    drupal_set_title("Список жанров");
    Global $sa, $rd; AnnChkbox();
    $gg = libListBlackGG();
    $CacheFile = 'cache/g/all'.($asql?'.anonim':'').".html";
    if ($fh = libOpenCacheFile($CacheFile, 24*3600)) {
      $sth = SELECT ("GenreId, GenreCode, GenreDesc, GenreMeta FROM libgenrelist ORDER BY 4,3");
      while ($g1 = dbf ($sth)) {
        $g = $g1->GenreCode;
        if ($g1->GenreMeta != $meta) {
          if ($meta) fwrite($fh, '</ul>');
          if (strstr($g, '_')) $g_all = substr($g, 0, strpos($g, '_'))."_all";
          else $g_all = $g.'_all';
          $meta = $g1->GenreMeta;
          if ($g_all != 'banking_all')
            fwrite($fh, "<h3>$meta</h3>\n<ul>\n");
          else 
            fwrite($fh, "<h3><a href=/g/$g_all/rss><img border=0 src=/misc/feed.png alt=RSS width=16 height=16></a>$meta</h3>\n<ul>\n");
        } 
        $n = Sel("Count(*) FROM libbook JOIN libgenre USING(BookId) WHERE GenreId=$g1->GenreId AND NOT (Deleted&1) $asql");
        if ($n) {
          fwrite($fh, "<li><a href=/g/$g/rss><img border=0 src=/misc/feed.png alt=RSS width=16 height=16></a> <a name=$g><a href=/g/$g>$g1->GenreDesc</a> ($g) - $n\n");
          if ($user->uid) fwrite($fh, "<a href=/g/$g1->GenreId/hide>(скрыть)</a>");
        }  
      }
      fclose($fh);
    }
    return libListBlackGenre().libReadCacheFile($CacheFile);
  } 

  if (arg(2) == 'rss') { 
    if ($p = strpos($g, '_all')) { 
      $g1 = substr($g, 0, $p)."%";
      return libRSS('g/'.$g, 1, $g, 'g', "JOIN libgenre USING(BookId) JOIN libgenrelist USING(GenreId)", "GenreCode LIKE '$g1' AND");
    }  
  }

  $r = "<h3><a href=/g>Список жанров</a></h3>";
  $g = addslashes($g);
  $g1 = S("* FROM libgenrelist WHERE GenreCode = '$g' OR GenreDesc = '$g' OR GenreId = '$g' LIMIT 1");
  if (!$g1) $g1 = S("* FROM libgenrelist WHERE GenreDesc like '%$g%' ORDER BY GenreId BY LIMIT 1");
  if (!$g1) return "Не существует жанра $g Извините.".$r;

  switch(arg(2)) {
    case rss:  
      return libRSS('g/'.$g1->GenreCode, 1, $g1->GenreDesc, "g/$g", "JOIN libgenre USING(BookId)", "GenreId = $g1->GenreId AND");
    case show:
      if ($user->uid) {
        $r = "<h2>Жанр $g1->GenreDesc удалён из чёрного списка.</h2>";
        Delete('libuserblackgenre WHERE UserId = %d AND GenreId = %d', $user->uid, $g1->GenreId);
      }  
      break;  
    case hide:
      if ($user->uid) {
        Insert('libuserblackgenre', 'UserId, GenreId', "$user->uid, $g1->GenreId");
        return "<h2>Жанр $g1->GenreDesc добавлен в чёрный список.</h2>".$r;
      }  
      break;  
    default:
      if ($user->uid) {
        if (Sel("GenreId FROM libuserblackgenre WHERE UserId = %d AND GenreId = %d", $user->uid, $g1->GenreId))
          return "<a href=/g/$g1->GenreId/show>Удалить жанр $g1->GenreDesc из чёрного списка</a>";
        $r = "<a href=/g/$g1->GenreId/hide>Добавить жанр '<b>$g1->GenreDesc</b>' в чёрный список</a>".$r;
      }
    break;        
  }
  
  drupal_set_title($g1->GenreDesc);
  drupal_set_html_head('<link rel="alternate" type="application/rss+xml" title="Новости жанра '.$g1->GenreDesc.' на Либрусеке RSS" href="/g/'.$g1->GenreCode.'/rss" />');  
  if (arg(2) == 'Title' ) {
    $order = 'Title';
    $c1 = 'checked';
    $oo='.t';
  } elseif (arg(2) == 'Time') {
    $order = '1 DESC';
    $c3 = 'checked';
    $oo='.b';
  } else {
    $c2 = 'checked';
    $joinseq = "LEFT JOIN libseq USING(BookId)";
    $order = "LastName, FirstName, MiddleName, SeqId, SeqNumb, Title";
  }  
  $CacheFile = "cache/g/$g1->GenreCode$oo$anonim.html";
  if ($fh = libOpenCacheFile($CacheFile, 24*3600)) {
    $sth = SELECT("libbook.BookId AS b, Time, Title, Title1, libavtor.AvtorId AS a FROM libbook JOIN libgenre USING(BookId) JOIN libavtor USING(BookId) JOIN libavtorname USING(AvtorId) $joinseq WHERE NOT (1&Deleted) AND GenreId = $g1->GenreId $asql ORDER BY $order");
    while ($a1 = dbf($sth)) { 
      if ($c3 && ($tm = ptm($a1->Time)))
          fwrite($fh, "<h4>$tm</h4>\n");  
      if (!$predb[$a1->b]) {
        $predb[$a1->b] = 1;
        $a = $a1->a;
        if ($preda != $a) fwrite($fh, "<h5>".avl($a)."</h5>"); 
        $preda = $a;
        $name = $a1->Title;
        if ($a1->Title1) $name .= " [$a1->Title1]";
        fwrite($fh, (++$i)." <a href=/b/$a1->b>$a1->Title</a><br>\n");
      }
    }  
    fclose($fh);
  }
  return $r."<form action=/g/$g>Сортировать
<input type=radio name=order value=a $c1 onClick='location.href=\"/g/$g/Title\"'>По алфавиту 
<input type=radio name=order value=b $c2 onClick='location.href=\"/g/$g\"'>По авторам
<input type=radio name=order value=t $c3 onClick='location.href=\"/g/$g/Time\"'>По дате поступления 
<ol>".libReadCacheFile($CacheFile)."</ol>";
}
