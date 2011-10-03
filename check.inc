<?php
module_load_include('inc', 'librusec');

Global $path, $PP;

function libCheck() {
  switch (arg(1)) { 
   case fixalias:
     $sth = SELECT ("* FROM libavtoraliase JOIN libavtor ON(AvtorId=BadId)");
     while ($a1 = dbf($sth)) {
       Update(libavtor, "AvtorId=$a1->GoodId", "AvtorId=$a1->BadId");
     }     
     $sth = SELECT ("* FROM libavtoraliase JOIN libtranslator  ON(translatorId=BadId)");
     while ($a1 = dbf($sth)) {
       Update(libtranslator, "translatorId=$a1->GoodId", "translatorId=$a1->BadId");
     }     
   case 'md5':
     $sth = SELECT("BookId, libfilename.FileName AS fn, md5 FROM libbook JOIN bookwarrior.updated USING(md5) JOIN libfilename USING(BookId) WHERE FileType <> 'fb2' ");
     while ($a1 = dbf($sth)) {
       $fn = "b.usr/".$a1->fn;
       $r .= "<li>".FileSize($fn)." ".$fn." ".bl($a1->BookId);
       Delete('libfilename WHERE BookId = %d', $a1->BookId);
       unlink($fn);
     }
     return $r;
     $r = "<ol>";
     if ($md5 = arg(2)) {
       $sth = SELECT("* FROM `libbook` WHERE md5 = '%s'", $md5);
       $bbb = '';
       while ($a1 = dbf($sth)) {
         $b = $a1->BookId;
         $r .= "<li>".DoLibRow($a1, 'nobreak')." <a href=/b/$b/delete>(удалить)</a>";
         if ($bbb) $r .= "<br><a href=/b/$bbb/join/$b>(объединить)</a>";
         else $bbb = $b;
         if ($a1->Deleted&1) {
           $r .= " (удалена)";
           RemoveBook($a1->BookId);
         } 
       }
     } else {
       $sth = SELECT("SUM(1) AS N, md5 FROM `libbook` GROUP BY 2 ORDER BY 1 DESC");
       while ($a1 = dbf($sth)) {
         if ($a1->N < 2) break;
         $r .= "<li>$a1->N <a href=/check/md5/$a1->md5>$a1->md5</a>";
         $sth1 = SELECT("* FROM `libbook` WHERE md5 = '%s'", $a1->md5);
         $b1 = $b2 = '';
         while ($a2 = dbf($sth1)) 
           if ($a2->Deleted&1) 
             RemoveBook($a2->BookId);
           elseif(!$b1) $b1 = $a2->BookId;
           else $b2 = $a2->BookId;
         $r .= " <a href=/b/$b1/join/$b2>(объединить)</a>";
       }
     }
   return $r."</ol>";

   case 'procallbooks':
     return procallbooks($qq);
     $f = fopen("cache/batch.log",'a+');
     fprintf($f, 'starts');
     fclose ($f);
     batch_set(array(
       'title' => t('Прочёсываем'),
       'operations' => array(array('procallbooks', array())),
        'finished' => 'my_finished_callback',
    'init_message' => t('Check Batch starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Batch 1 has encountered an error.'),
         'file' => drupal_get_path('module', 'librusec') . '/check.inc',    
     ));
   batch_process();
   return 'batched';

    case libCheckLitres: 
       return libCheckLitres(0);
    case massfail:
      $fh = fopen('massfail', 'r');
      while ($s = fgets($fh)) {
        preg_match('/b.tmp.(\d+).zip/', $s, $m);
        $IDS[$m[1]]++;
      }
      print "<pre>";
      foreach ($IDS as $k => $v) {
        if ($v > 10) {
          $filename = "b.tmp/$k.zip";
          $bb = explode('.', Sel("Books FROM libmass WHERE ID = %d", $k));
          $filelist = '';
          for ($i = 1; $b = $bb[$i]; $i++) {
             $zip = MakeZipFile($b, $bb[0]);
             print "$zip = MakeZipFile($b, $bb[0]);\n";
             if (file_exists($zip)) $filelist .= " \"$zip\"";
          }
          print system("zip -j -0 $filename $filelist\n"); 
        }
      }
      exit;
    case createtorrent:
      print createtorrent;
      $b = (int)arg(2);
      CreateTorrent($b);
      exit;
    break;
    case torrents: {
      require_once('modules/bittorrent/includes/bt_common.inc');
      foreach(glob('all/*torrent') as $file) {
        print $file;
        if ($torrent = bdecode_file($file)) {
          print " decoded<br>";
//          INSERT(bt_torrents, "nid, fid, info_hash, metadata", "136104, 136, '%s', %b", info_hash($torrent), bencode($torrent));
          INSERT(bt_torrents, "nid, fid, info_hash", "136104, 136, '%s'", info_hash($torrent));
        } 
      }
    } 
    exit;
    case splittit:
      $sth = SELECT("* FROM `libbook` WHERE `Title` LIKE '%[%]%' AND Title1 = ''");
      while ($a1 = dbf($sth)) {
        $b = $a1->BookId;
        $r .= bl($b)."<br>";
        if (preg_match('/(.+)\s*\[(.+)\].*/', $a1->Title, $m)) {
          if ($m[1] && $m[2] &&!$m[3]) {
            UPDATE(libbook, "Title='%s', Title1='%s'", "BookId=%d", $m[1], $m[2], $b);
          }  
        }
      }
      return $r;
    case db:
      return libdb();
    case schema:
      return libCheckSchema(arg(2), arg(3));
    case topadv:
      return checktopadv(arg(2)); 
    case read:
      return libNewRead(arg(2));
    case badnames:
      $sth = select("* from libfilename JOIN libbook USING (BookId) where filename like '%?%'");
      while ($a1 = dbf($sth)) {
        $r .= "<li><a href=/b/$a1->BookId>$a1->Title</a>";
      }
      return $r;
    case check:
    $b = 123456;
    return  'lb'.((int)($b/100));
    print session_name(); exit;
$rubr = array('transport', 'transport', 'sport', 'milym-damam', 'milym-damam', 'milym-damam', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'detjam', 'avto-i-moto', 'puteshestvija-i-turizm', 'puteshestvija-i-turizm', 'puteshestvija-i-turizm', 'biznes-v-internete', 'biznes-v-internete', 'biznes-v-internete', 'biznes-v-internete', 'smi', 'obrazovanie-i-nauka', 'obrazovanie-i-nauka', 'dosug', 'uslugi', 'banki-i-strahovanie', 'banki-i-strahovanie', 'stroitelstvo-i-remont', 'stroitelstvo-i-remont', 'stroitelstvo-i-remont', 'stroitelstvo-i-remont', 'nedvizhimost', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny'); 
$rr = '/'.$rubr[rand(0, count($rubr)-1)].rand(count($rubr)-1);
print $rr;
print rand(0, count($rubr));
 exit;   
    
    case libcg:
      print "<pre>";
      $s = libcg(arg(2));
      print_r($s);
    exit;
    
   case SetActionBookId: 
     return SetActionBookId();

   case delnul:
      foreach (array(109324) as $b) {
        $r .= $b.' ';
        RemoveBook($b);
      }  
      return $r;
      $sth = SELECT ("BookId FROM libbook WHERE FileSize = 0");
      while ($a1=dbf($sth)) 
        RemoveBook($a1->BookId);
break;  
    case SERVER: 
      print "<pre>"; print_r ($_SERVER); print "</pre>";
      phpinfo(); 
    exit;
    case setdocauthor:
      $sth = SELECT("* FROM libbook WHERE FileAuthor = '' AND FileType = 'fb2' LIMIT 12345");
      while ($a1=dbf($sth)) {
        Global $path, $PP; $path = array(); $PP = '';
        $xml_parser = xml_parser_create();
        xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
        xml_set_element_handler($xml_parser, f1, f2);
        xml_set_character_data_handler($xml_parser, f3);
        if (!$fp = fopen($fb2 = "b/$b.fb2", "r")) 
           return "Ошибочка: не открылся файлик $fb2 ".bl($a1->BookId); 
        xml_parse($xml_parser, $d=fread($fp, 2000), 1);
        xml_parser_free($xml_parser); 
        $nick = $PP->nick;
        if (!$nick) $nick = "$PP->fn $PP->ln";
        if (!$nick) $nick = $PP->prog;
        print "<li>$a1->BookId:$nick\n"; $i++;
        if ($nick) UPDATE('libbook', "FileAuthor = \"$nick\"", "BookId = $a1->BookId");
      }
    print "<br>$i files processed";
    exit;  
    case lang:
      $sth = SELECT("BookId FROM libbook WHERE lang <> 'ru' AND FileType = 'fb2' LIMIT 22222"); 
      while ($a1=dbf($sth)) {
        $fh = fopen("b/$b.fb2", 'r');
        $Data = fread($fh, 4000);
        fclose($fh);
        if (preg_match('/<lang>(..)<.lang>/', $Data, $m)) {
          Update ('libbook', "Lang = '$m[1]'", "BookId = $a1->BookId");
        }
        $a2 = $a1;
      }
      print "<pre>";
      print_r($a2);
      print_r($m);
    exit;
    case gg:
      for ($i=2000; $i <=10000; $i++) 
       if (!Sel ("GenreId FROM libgenre WHERE BookId = $i")) 
         db_query("INSERT INTO libgenre (BookId, GenreId) VALUES ($i, 12)");
    break;      
    case da:
      $sth = SELECT("sum(1) as N, BookId, AvtorId FROM `libavtor` group by 2,3 order by 1 desc");
      while ($a1=dbf($sth)) {
        if ($a1->N < 2) return 'OK';
        db_query("DELETE FROM libavtor WHERE BookId = $a1->BookId AND AvtorId = $a1->AvtorId LIMIT ".($a1->N - 1));
      }
      exit;
    case dg:
      $sth = SELECT("sum(1) as N, BookId, GenreId FROM `libgenre` group by 2,3 order by 1 desc");
      while ($a1=dbf($sth)) {
        if ($a1->N < 2) return 'OK';
        db_query("DELETE FROM libgenre WHERE BookId = $a1->BookId AND GenreId = $a1->GenreId LIMIT ".($a1->N - 1));
      }
      exit;
    case rate:
      $sth = SELECT ("* FROM old2.liblog WHERE rate > 0");
      while ($a1=dbf($sth)) {
        $c1++;
        $a2 = S("*  FROM liblog WHERE UserId = $a1->UserId AND BookId = $a1->BookId");
        if ($a2->ID) {
          $c2++;
          if (!$a2->Rate)  //читали не оценили
            db_query("UPDATE liblog SET Rate = $a1->Rate WHERE UserId = $a1->UserId AND BookId = $a1->BookId");
        } else {
          $c3++;
          db_query("INSERT INTO liblog (UserId, BookId, Time, Rate) VALUES ($a1->UserId, $a1->BookId, '$a1->Time', $a1->Rate)");
        }     
      }      
      return "$c1 $c2 $c3";

    case 'bdesc':
      $sth = SELECT ("* FROM node join `node_revisions` using (nid) WHERE type = 'bdesc' AND body = '' AND teaser = ''");
      while ($a1 = dbf($sth)) {
       db_query('DELETE FROM {node} WHERE nid = %d', $a1->nid);
       db_query('DELETE FROM {node_revisions} WHERE nid = %d', $a1->nid);
       $r .= "<li><a href=/node/$a1->nid>$a1->title</a>";
      }
    return $r;
    case litres_genre_list:
      $xml = simplexml_load_file('http://www.litres.ru/genres_list/');
      if (!$xml) return "error loading xml"; 
      print('<pre>');
      foreach ($xml->genre as $mg) {
        $meta = $mg->attributes()->title[0];
        foreach ($mg->subgenres as $g) {
          $gname = $g->attributes()->title;
          $gcode = $g->attributes()->token;
          if (!Sel("GenreCode FROM libgenrelist WHERE GenreCode = '$gcode'")) {
            Insert (libgenrelist, 'GenreCode, GenreDesc, GenreMeta', "'$gcode', '$gname', '$meta'");
            print "\n$gcode $gname $meta ADDED";
          }  
        }
      }
      return;
    case 'xml':
      $sth = pager_query("SELECT BookId FROM libbook WHERE FileType = 'fb2' ORDER BY 1", 1000);
      while ($a1 = dbf($sth)) {
        $b = $a1->BookId;
        $xml = simplexml_load_file("b/$b.fb2");
        if (!$xml) $r .= "<li>$b <pre>".print_r(libxml_get_last_error(),1)."</pre>";
     }
    case memcache:
      print "<pre>";
      global $_memcache_statistics;
      print_r($_memcache_statistics);
      exit; 
    case libcachedel:
          dmemcache_delete("librate".arg(2), librusec);
     break;
    case libcache: 
      if (!arg(2)) {
        $hits = apc_fetch('Dolibrow Hits');
        $all = apc_fetch('Dolibrow All');
        $add = apc_fetch('Dolibrow Added');
        $per1 = ((int)($hits/($add + $hits)*100)).'%';
        $skip = $all - $add - $hits;
        $per2 = ((int)($skip/($all)*100)).'%';
        print "<p>Dolibrow hits: $hits ($per1), added $add, skip $skip ($per2), all $all";
        $hits = apc_fetch('Search hit');
        $add = apc_fetch('Search stored');
        $per = ((int)($hits/($hits+$add)*100)).'%';
        print "<p>Search hits: $hits ($per), added $add";
        exit;
      }  
      $c = cache_get(arg(2), librusec);
      if ($c) {print "Cache:<pre>"; print_r($c); exit;}
      $c = cache_get("librate".arg(2), librusec);
      if ($c) {print "RATE<pre>"; print_r($c); }
      else print "no cache for user ".arg(2);
      $c = cache_get($cid = "libwst".arg(2), librusec);
      if ($c) {print "WST<pre>"; print_r($c); }
      else print "\n\n<p>no cache for book ".arg(2);
     exit;
      
    case wst: return libWST(arg(2));
   return $r.theme('pager');
  }
  return 'не понял задачи';
}

//SELECT BookId, sum(1) AS N FROM `liblog` WHERE substr(time,3,5) = '08-12' GROUP BY 1

function f1($parser, $E, $attrs) { Global $path;  $path[] = strtolower($E);   }
function f2($parser, $E) { Global $path;  array_pop($path);}
function f3($parser, $E) {
  Global $path, $PP;  
  $p = join('/',$path);
  if ($p == 'fictionbook/description/document-info/author/nickname') $PP->nick .= $E;
  if ($p == 'fictionbook/description/document-info/author/first-name') $PP->fn .= $E;
  if ($p == 'fictionbook/description/document-info/author/last-name') $PP->ln .= $E;
  if ($p == 'fictionbook/description/document-info/program-used') $PP->prog .= $E;
}

function procallbooks(&$context) {
  $f = fopen("cache/batch.log",'a+');
  $b = $context['sandbox'];
  if (!$b) $b = 1;
  $maxb = Sel('MAX(BookId) FROM libbook');
  $sth = SELECT("* FROM libbook WHERE md5 = '' LIMIT 1000");
  while ($a1 = dbf($sth)) {
    $b = $a1->BookId;
    fprintf($f, "$b:$b->FileType ");
    if ($a1->FileType == 'fb2') {
      $fn = "b/$b.fb2";
    } else {  
      $fn = "b.usr/".Sel("FileName FROM libfilename WHERE BookId = %d", $b);
    }  
    if (!file_exists($fn) || filesize($fn) < 2000) {
      RemoveBook($b);
      fprintf($f, "$b removed - file $fn not exist\n");
      $context['message'] = "$b removed - file $fn does not exists\n";
      $context['finished'] = 1;
      return;
    } else {
      fprintf($f, "$fn found");
    }
    $md5 = md5_file($fn);
    if (!$md5) {
      fprintf($f, " - bad md5\n");
      continue;
    }
    Update('libbook', "md5='%s'", 'BookId=%d', $md5, $b);
    if ($a1->FileType != 'fb2') {
      fprintf($f, "\n");
      continue; // остальное только для фб2
    }  
    $xml = simplexml_load_file($fn);        
    if (!$xml) { //битый xml, отмечаем в базе для отмены конверторов
      fprintf($f, " - bad xml\n");
      Update('libbook', "Broken=1", 'BookId=%d', $b);
      continue;
    }   
    $titinfo = $xml->description->{'title-info'};
    $srclang = $titinfo->{'src-lang'};
    $lang = $titinfo->{'lang'};
    if ($srclang && $lang !== $srclang) 
        Insert(libsrclang, 'BookId, SrcLang', "%d, '%s'", $b, $srclang);  
    if ($keywords = $titinfo->{'keywords'})     
      Update('libbook', "keywords='%s'", 'BookId=%d', $b, $keywords);
    unset($xml);  
    $context['message'] = $context['sandbox'] = $b + 1;
    $context['results'][a]++;
    fprintf($f, " \n");
  }
  $context['finished'] = $b/$maxb;
  fprintf($f,print_r($context, 1));
  fprintf($f, "\n");
  fclose ($f);
}
  
function find_translators(&$context) {
  return;
  $f = fopen("cache/batch.log",'a+');
  $b = $context['sandbox'];
  if (!$b) $b = 1;
  $context['finished'] = $b/138500;
  $sth = SELECT("* FROM libbook WHERE FileType = 'fb2' AND BookId > $b AND NOT (Deleted&1) ORDER BY BookId LIMIT 50");
  while ($a1 = dbf($sth)) {
    $b = $a1->BookId;
    fprintf($f, " $b ");
    if (Sel ("TranslatorId FROM libtranslator WHERE BookId = $b")) continue;
    if ($e = parsefb2("b/$b.fb2", $b, 'head')) { fprintf($f, $e);exit;}
    Global $PD;
    if ($PD->TranslatorIds)
    foreach ($PD->TranslatorIds as $aid) {
      Insert ('libtranslator', 'BookId, TranslatorId', "$b, $aid"); 
      fprintf($f, " ($b.$aid) ");
      $context['results'][a]++;
    }  
    $context['message'] = $context['sandbox'] = $b + 1;
    $context['message'] .= "/".$context['results'][a];
  }
  fprintf($f, "\n");
  fclose ($f);
}

function my_finished_callback($success, $results, $operations) {
  $f = fopen("cache/batch.log",'a+');
  fprintf($f, "success, results, operations - $success, $results, $operations");
  fclose ($f);
}

function SetActionBookId() {
  print "<pre>";  
  $sth = SELECT ("* FROM libbook LEFT JOIN libactions USING(BookId) WHERE BookId > 60000 AND ISNULL(ActionId) ORDER BY `libbook`.`BookId` DESC LIMIT 10");
  while ($a1 = dbf($sth)) {
    $id = Sel("ActionId FROM `libactions` WHERE `ActionUndo` = '' AND `BookId` = 0 AND ActionSQL like '%s' ORDER BY ActionId DESC LIMIT 1", 
                 "%$a1->FileSize%$a1->FileType%$a1->Title%$a1->Id%");
    if (!$id)             
      $id = Sel("ActionId FROM `libactions` WHERE `ActionUndo` = '' AND `BookId` = 0 AND ActionSQL like '%s' ORDER BY ActionId DESC LIMIT 1", 
                 "%$a1->FileSize%$a1->FileType%$a1->Id%");
    if ($id) {
      Update(libactions, "BookId = $a1->BookId", "ActionId=$id");
      //$q = S("* FROM libactions WHERE ActionId=$id"); print_r($q);exit;
      print "\n".bl($a1->BookId);
    } else {
      print "\n";
      print_r($a1);
      $id = Sel("ActionId FROM `libactions` WHERE `ActionUndo` = '' AND ActionSQL like '%s' ORDER BY ActionId DESC LIMIT 1", 
                 "%$a1->FileSize%$a1->FileType%$a1->Title%$a1->Id%");
      if ($id) {
        $q = S("* FROM libactions WHERE ActionId = $id");
        print "Занят под $id : ".print_r($q);
      } else {
        print "Book Fail ".bl($a1->BookId);
        print "\n"."ActionId FROM `libactions` WHERE `ActionUndo` = '' AND `BookId` = 0 AND ActionSQL like '%$a1->FileSize%$a1->FileType%$a1->Title%$a1->Id%' ORDER BY ActionId DESC ". 
                 "";
      }              
    }  
  } 
exit;
}

function libNewRead($b) {
  $html = "/tmp/qq.html";
  if ($err = newparsefb2($b, 'htm', $html)) return $err;
  readfile($html);
  exit;
}

function libCheckSchema($b1, $b2) {
  include_once drupal_get_path('module', 'librusec').'/validator.inc';
  if ($b2 > $b1) {
    $sth = SELECT("BookId FROM libbook WHERE NOT Deleted&1 AND BookId >= $b1 AND BookId <= $b2");
    while ($a1 = dbf($sth)) {
      $b = $a1->BookId;
      $r .= "<li><a href=/b/$b>$b</a> ".validate("b/$b.fb2");
    }  
  } else {
    foreach (glob("2do/*") as $fb2) 
      if ($i++>1000*($b1-1) && $i<=1000*$b1)
        $r .= "<li>$fb2 ".validate($fb2);
  }  
  return "<ol>$r</ol>";  
}


function libdb() {
  $book = new DB;
  print $book->Title(1);
  exit;
}

function CreateTorrent($b) {
  $dir = 'b.usr/';
  $fn = Sel("FileName FROM libfilename WHERE BookId = %d", $b);
  if (!$fn) return 0;
  chdir ($dir);
  if (!file_exists($fn)) return 0;
#  print `/usr/bin/maketorrent-console --target librusec.$b.torrent http://torrent.rus.ec/bt/ $fn`;
  chdir("/www");
  $torent = 'librusec.'.$b.'.torrent';
  rename($dir.$torent, 'torrents/'.$torent);
  print "  rename($dir$torent, 'torrents/'.$torent);";
}

function dbtuning_get_mysql_version() {
/*
Grab the Mysql Version for use in forming the help page URLs
*/
$version_object = db_fetch_object( db_query('SHOW VARIABLES LIKE "version"'));
  if (substr($version_object->Value, 0, 3) == 4.1) {
    $mysql_version = substr($version_object->Value, 0, 3);
  }
    elseif (substr($version_object->Value, 0, 1) == 4) {
      $mysql_version = substr($version_object->Value, 0, 1);
    }
      elseif (substr($version_object->Value, 0, 1) == 5) {
        $mysql_version = substr($version_object->Value, 0, 1);
      }
  return $mysql_version;
}
function dbtuning_hit_ratio_monitor() {
  echo "<tr><td><b>" . t('Threads Created') . "</b></td><td><b>" . t('Connections') . "</b></td><td><b>" . t('Thread Cache Size') . "</b></td><td><b>" . t('Hit Ratio') . "</b></td><td><b>" . t('Warnings') . "</b></td></tr>";
  $threads_created = db_fetch_object( db_query('SHOW STATUS LIKE "Threads_created"'));
  $thread_cache_size = db_fetch_object( db_query('SHOW VARIABLES LIKE "thread_cache_size"'));
  $mysql_connections = db_fetch_object( db_query('SHOW STATUS LIKE "Connections"'));
  $hit_ratio = (((int) $threads_created->Value / (int) $mysql_connections->Value));
  if ($hit_ratio < 0.90 || $hit_ratio > 1.10) {
    $error_value = t('The ideal situation is to get Threads Created as close as possible to Thread Cache Size. So no new connections are having to wait for new thread allocation. Stay as close to a 99% hit ratio as you can as this will reduce bottlenecks in your caching. Adjust your Thread Cache Size until this is achieved. You can set your Thread Cache Size on the fly by doing "SET GLOBAL thread_cache_size=N". Where N is the desired size of your Thread Cache. For additional information on the SET command please read <a href="/%url">SET Syntax</a>', array('%url' => url('http://dev.mysql.com/doc/refman/5.1/en/set-option.html', NULL, NULL, TRUE)));
  }
    else {
      $error_value = "";
    }
  echo "<tr><td>" . $threads_created->Value . "</td><td>" . $mysql_connections->Value . "</td><td>" . $thread_cache_size->Value . "</td><td>" . (round($hit_ratio, 3) * 100) . "%</td><td>" . $error_value . "</td></tr>";
}
function dbtuning_calc_uptime_stats($stats_to_calc) {
  $uptime = db_fetch_object( db_query('SHOW STATUS LIKE "Uptime"'));
    $stat_hour = ($stats_to_calc->Value / ($uptime->Value / 3600));
    $stat_day = ($stats_to_calc->Value / ($uptime->Value / 86400));
    $stat_year = ($stats_to_calc->Value / ($uptime->Value / 31536000));
    echo "<tr><td>" . $stats_to_calc->Variable_name . "</td><td>" . $stat_hour . "</td><td>" . $stat_day . "</td><td>" . $stat_year . "</td><td>" . $stats_to_calc->Value . "</td></tr>\n";
}
function dbtuning_uptime_stats() {
  echo "<tr><td><b>" . t('Database Action') . "</b></td><td><b>" . t('Per-Hour') . "</b></td><td><b>" . t('Per-Day') . "</b></td><td><b>" . t('Per-Year') . "</b></td><td><b>" . t('Total') . "</b></td></tr>";
  $results_load_stats = db_query('SHOW STATUS LIKE "Handler%"');
    while ($stats = db_fetch_object($results_load_stats)) {
      switch ($stats) {
        case ($stats->Variable_name == 'Handler_write'):
          $stats->Variable_name = t("Writes to DB");
          dbtuning_calc_uptime_stats($stats);
          break;
        case ($stats->Variable_name == 'Handler_update'):
          $stats->Variable_name = t("Updates to DB");
          dbtuning_calc_uptime_stats($stats);
          break;
        case ($stats->Variable_name == 'Handler_delete'):
          $stats->Variable_name = t("Deletes from DB");
          dbtuning_calc_uptime_stats($stats);
          break;
        default:
          break;
    }
  }
}
function dbtuning_table_stats() {
  echo "<tr><td><b>" . t('Table Cache') . "</b></td><td><b>" . t('Open Tables') . "</b></td><td><b>" . t('Opened Tables') . "</b></td><td><b>" . t('Warnings') . "</b></td></tr>";
  $open_tables = db_fetch_object( db_query('SHOW STATUS LIKE "Open_tables"'));
  $opened_tables = db_fetch_object( db_query('SHOW STATUS LIKE "Opened_tables"'));
  $uptime = db_fetch_object( db_query('SHOW STATUS LIKE "Uptime"'));
  $table_cache = db_fetch_object( db_query('SHOW VARIABLES LIKE "table_cache"'));
  switch ($table_cache) {
    case ($uptime < 1000000):
      $error_msg = t("Your MySQL server has not been running long enough to make a quality assessment of the performance of your table cache. Put some traffic on there and come back soon!");
      echo "<tr><td>" . $table_cache->Value . "</td><td>" . $open_tables->Value . "</td><td>" . $opened_tables->Value . "</td><td>" . $error_msg . "</td><td>Uptime: " . $uptime->Value . "</td></tr>";
      break;
    case ($table_cache->Value == $open_tables->Value && $opened_tables->Value > 1000):
      $error_msg = t("Your table cache is currently full. This can severely impact the performance of your MySQL server. If you have the memory, it may be time to increase your table cache. However, if your table cache is set too high, MySQL may start dropping connections. You can read about how MySQL uses the table cache <a href='%url'>here</a>. Increase your thread cache by issuing a 'SET thread_cache_size=N' command. Where N is the desired size of your Thread Cache. More Information on the SET command can be found <a href='%set'>here</a>.", array('%set' => url('http://dev.mysql.com/doc/refman/5.0/en/set-option.html', NULL, NULL, TRUE)), array('%url' => url('http://dev.mysql.com/doc/refman/5.0/en/table-cache.html', NULL, NULL, TRUE))) ;
      echo "<tr><td>" . $table_cache->Value . "</td><td>" . $open_tables->Value . "</td><td>" . $opened_tables->Value . "</td><td>" . $error_msg . "</td></tr>";
      break;
    case ($table_cache->Value == $open_tables->Value && $opened_tables->Value < 1000):
      $error_msg = t("Your table cache is currently full. Normally this can be bad as it forces MySQL to goto the DB for queries. However, due to the low number of opened_tables, it may provide little benefit to increase your table cache");
      echo "<tr><td>" . $table_cache->Value . "</td><td>" . $open_tables->Value . "</td><td>" . $opened_tables->Value . "</td><td>" . $error_msg . "</td></tr>";
      break;
    case (($table_cache->Value - $open_tables->Value) > 10):
      $error_msg = t("If MySQL is using a significant amount of resources your system. You may want to free up memory by lowering your Table Cache. You can read about how MySQL uses the table cache <a href='%url'>here</a>. Adjust your thread cache by issuing a 'SET GLOBAL thread_cache_size=N' command. Where N is the desired size of your Thread Cache. More Information on the SET command can be found <a href='%set'>here</a>.", array('%set' => url('http://dev.mysql.com/doc/refman/5.0/en/set-option.html', NULL, NULL, TRUE)), array('%url' => url('http://dev.mysql.com/doc/refman/5.0/en/table-cache.html', NULL, NULL, TRUE))) ;
      echo "<tr><td>" . $table_cache->Value . "</td><td>" . $open_tables->Value . "</td><td>" . $opened_tables->Value . "</td><td>" . $error_msg . "</td></tr>";
      break;
    default:
      $error_msg = "";
      echo "<tr><td>" . $table_cache->Value . "</td><td>" . $open_tables->Value . "</td><td>" . $opened_tables->Value . "</td><td>" . $error_msg . "</td></tr>";
      break;
  }
}
function dbtuning_qcache_stats_calc($qcache_vars) {
if ($qcache_vars['ratio'] != 0) {
  if (round($qcache_vars['ratio'], 2) > .10) {
    $qcache_errs['ratio'] = t('Your query cache is not maintaining a healthy hit ratio of 1 / 10 - Inserts / Hits. This can mean that your mySQL instance has become hard drive bound. This statistic can be improved by using your mySQL thread, table, and query cache effectively. You can also improve this from a code standpoint. Maintain a level of consistency in your code. Whether or not a query will be placed in the cache is case sensitive, as mySQL checks for a byte identical match.');
      }
  }
    else {
      $qcache_errs['ratio'] = "Your Query Cache is disabled.";
    }
  if ($qcache_vars['litmus']->Value == 'NO') {
    $qcache_errs['litmus'] = t("Your Query Cache has been disabled. The Query Cache is useful for when you regularly serve the same exact page. Please recompile MySQL without the --without-query-cache configuration option. See <a href='%url'>here</a> for more information on compiling MySQL.", array('%url' => url('http://dev.mysql.com/doc/refman/5.0/en/installing-source.html', NULL, NULL, TRUE)));
  }
  if ($qcache_vars['size']->Value == 0) {
    $qcache_errs['size'] = t("Your query_cache_size is set to 0. This effectively disables your Query Cache. To correct this use the command '<i>SET GLOBAL query_cache_size=N</i>' where N is equal to the desired size of your Query Cache. More Information on the SET command can be found <a href='%set'>here</a> and more information on configuring your Query Cache can be found <a href='%qcache'>here</a>", array('%set' => url('http://dev.mysql.com/doc/refman/5.0/en/set-option.html', NULL, NULL, TRUE)), array('%qcache' => url('http://dev.mysql.com/doc/refman/5.0/en/query-cache-configuration.html', NULL, NULL, TRUE)));
  }
  if ($qcache_vars['type']->Value == 'OFF') {
    $qcache_errs['type'] = t("Your query_cache_type is set to 0. This effectively disables your Query Cache. The Query Cache is useful for when you regularly serve the same exact page. To correct this use the command 'SET GLOBAL query_cache_type=N' where N is equal to OFF,ON, or DEMAND. More Information on the SET command can be found <a href='%set'>here</a> and more information on configuring your Query Cache can be found <a href='%qcache'>here</a>", array('%set' => url('http://dev.mysql.com/doc/refman/5.0/en/set-option.html', NULL, NULL, TRUE)), array('%qcache' => url('http://dev.mysql.com/doc/refman/5.0/en/query-cache-configuration.html', NULL, NULL, TRUE)));
  }
  if ($qcache_vars['free_blocks']->Value > 100) {
    $qcache_errs['free_blocks'] = t('If your Query Cache has a lot of free blocks in it. This can indicate fragmentation in your Query Cache. This can decrease performance and generally just be a waste of resoures. Fix this problem by issuing a \'FLUSH QUERY CACHE\' command. Read more about query cache maintenance <a href="/%url">here</a>.', array('%url' => url('http://dev.mysql.com/doc/refman/5.0/en/query-cache-status-and-maintenance.html', NULL, NULL, TRUE)));
  }
return $qcache_errs;
}
function dbtuning_qcache_gather_vars() {
  $qcache_vars = array (
  "uptime" => db_fetch_object( db_query('SHOW STATUS LIKE "Uptime"')),
  "hits" => db_fetch_object( db_query('SHOW STATUS LIKE "qcache_hits"')),
  "inserts" => db_fetch_object( db_query('SHOW STATUS LIKE "qcache_inserts"')),
  "litmus" => db_fetch_object( db_query('SHOW VARIABLES LIKE "have_query_cache"')),
  "size" => db_fetch_object( db_query('SHOW VARIABLES LIKE "query_cache_size"')),
  "type" => db_fetch_object( db_query('SHOW VARIABLES LIKE "query_cache_type"')),
  "free_memory" => db_fetch_object( db_query('SHOW STATUS LIKE "qcache_free_memory"')),
  "free_blocks" => db_fetch_object( db_query('SHOW STATUS LIKE "qcache_free_blocks"')),
  "lowmem_prunes" => db_fetch_object( db_query('SHOW STATUS LIKE "qcache_lowmem_prunes"')),
  "limit" => db_fetch_object( db_query('SHOW VARIABLES LIKE "query_cache_limit"')),
  "not_cached" => db_fetch_object (db_query('SHOW STATUS LIKE "qcache_not_cached"')));
    if ($qcache_vars['inserts']->Value == 0 || $qcache_vars['hits']->Value == 0) {
      $qcache_vars['ratio'] = 0;
    }
      else {
        $qcache_vars['ratio'] = round(($qcache_vars['inserts']->Value / $qcache_vars['hits']->Value), 2);
      }
  return $qcache_vars;
}
function dbtuning_qcache_stats() {
  $qcache_vars = dbtuning_qcache_gather_vars();
  $qcache_errs = dbtuning_qcache_stats_calc($qcache_vars);
  echo "<tr><td><b>" . t('Query Cache Ratio') . "</b></td><td>" . $qcache_vars['ratio'] . "</td><td>" . $qcache_errs['ratio'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Inserts') . "</b></td><td>" . $qcache_vars['inserts']->Value . "</td><td>" . $qcache_errs['inserts'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Hits') . "</b></td><td>" . $qcache_vars['hits']->Value . "</td><td>" . $qcache_errs['hits'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Available?') . "</b></td><td>" . $qcache_vars['litmus']->Value . "</td><td>" . $qcache_errs['litmus'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Size') . "</b></td><td>" . $qcache_vars['size']->Value . "</td><td>" . $qcache_errs['size'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Type') . "</b></td><td>" . $qcache_vars['type']->Value . "</td><td>" . $qcache_errs['type'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Free Memory') . "</b></td><td>" . $qcache_vars['free_memory']->Value . "</td><td>" . $qcache_errs['free_memory'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Free Blocks') . "</b></td><td>" . $qcache_vars['free_blocks']->Value . "</td><td>" . $qcache_errs['free_blocks'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Prunes') . "</b></td><td>" . $qcache_vars['lowmem_prunes']->Value . "</td><td>" . $qcache_errs['lowmem_prunes'] . "</td></tr>";
  echo "<tr><td><b>" . t('Query Cache Limit') . "</b></td><td>" . $qcache_vars['limit']->Value . "</td><td>" . $qcache_errs['limit'] . "</td></tr>";
  echo "<tr><td><b>" . t('Queries Not Cached') . "</b></td><td>" . $qcache_vars['not_cached']->Value . "</td><td>" . $qcache_errs['not_cached'] . "</td></tr>";
}
function dbtuning_everything_else() {
  $mysql_version = dbtuning_get_mysql_version();
  $results = db_query('SHOW STATUS');
  echo "<tr><td><b>" . t('Variable Name') . "</b></td><td><b>" . t('Current Value') . "</b></td><td><b>" . t('Warnings') . "</b></td></tr>";
  while ($node = db_fetch_object($results)) {
    switch ($node) {
      case ($node->Value > 0 && $node->Variable_name == 'Slow_queries'):
        $value_error = t("You Have queries which are executing slower than normal. Enable the <a href='%url2'>Slow Query Log</a> and use <a href='%url'>Explain</a> to examine your queries.", array('%url' => url('http://dev.mysql.com/doc/refman/en/slow-query-log.html', NULL, NULL, TRUE)), array('%url2' => url('http://dev.mysql.com/doc/refman/en/explain.html', NULL, NULL, TRUE)));
        echo "<tr><td>" . $node->Variable_name . "</td><td>" . $node->Value . "</td><td>" . $value_error . "</td></tr>\n";
        break;
      case ($node->Variable_name == 'Select_scan'):
        $value_error = t("A high value here can be an indication of bottlenecks in your server optimization. This happens because Mysql is not using the indexes for the tables and so is having to do extra work for inefficient queries. Enable the <a href='%url'>Slow Query Log</a> and use <a href='%url2'>Explain</a> to examine your queries.", array('%url' => url('http://dev.mysql.com/doc/refman/en/slow-query-log.html', NULL, NULL, TRUE)), array('%url2' => url('http://dev.mysql.com/doc/refman/en/explain.html', NULL, NULL, TRUE)));
        echo "<tr><td>" . $node->Variable_name . "</td><td>" . $node->Value . "</td><td>" . $value_error . "</td></tr>\n";
        break;
      case ($node->Variable_name == 'Select_full_join'):
        $value_error = t("A high value here means that MySQL is not using indexes and is therefore taking longer to build a result set. The problem can be fixed by indexing important fields of the join.Enable the <a href='%url'>Slow Query Log</a> and use <a href='%url2'>Explain</a> to examine your queries.", array('%url' => url('http://dev.mysql.com/doc/refman/en/slow-query-log.html', NULL, NULL, TRUE)), array('%url2' => url('http://dev.mysql.com/doc/refman/en/explain.html', NULL, NULL, TRUE)));
        echo "<tr><td>" . $node->Variable_name . "</td><td>" . $node->Value . "</td><td>" . $value_error . "</td></tr>\n";
        break;
      default:
        break;
    }
  }
}
function dbtuning_memory_check() {
/*
key_buffer + innodb_buffer_pool + innodb_log_buffer + innodb_additional_mem_pool + net_buffer + max_connections *
(read_buffer + join_buffer + sort_buffer + myisam_sort_buffer + thread_stack + tmp_table_size + read_rnd_buffer)
*/
  $memory_vars = array (
  "innodb_buffer_pool" => db_fetch_object( db_query('SHOW VARIABLES LIKE "innodb_buffer_pool_size"')),
  "innodb_additional_mem_pool" => db_fetch_object( db_query('SHOW VARIABLES LIKE "innodb_additional_mem_pool_size"')),
  "innodb_log_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "innodb_log_buffer_size"')),
  "thread_stack" => db_fetch_object( db_query('SHOW VARIABLES LIKE "thread_stack"')),
  "tmp_table_size" => db_fetch_object( db_query('SHOW VARIABLES LIKE "tmp_table_size"')),
  "key_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "key_buffer_size"')),
  "net_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "net_buffer_length"')),
  "max_connections" => db_fetch_object( db_query('SHOW VARIABLES LIKE "max_connections"')),
  "sort_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "sort_buffer_size"')),
  "myisam_sort_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "myisam_sort_buffer_size"')),
  "read_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "read_buffer_size"')),
  "join_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "join_buffer_size"')),
  "read_rnd_buffer" => db_fetch_object( db_query('SHOW VARIABLES LIKE "read_rnd_buffer_size"')));
  $global_buffer = ((floatval($memory_vars['read_buffer']->Value) + floatval($memory_vars['join_buffer']->Value) + floatval($memory_vars['sort_buffer']->Value) + floatval($memory_vars['myisam_sort_buffer']->Value) + floatval($memory_vars['thread_stack']->Value) + floatval($memory_vars['tmp_table_size']->Value) + floatval($memory_vars['read_rnd_buffer']->Value)));
  $per_thread_mem = ((floatval($memory_vars['key_buffer']->Value) + floatval($memory_vars['innodb_buffer_pool']->Value) + floatval($memory_vars['innodb_log_buffer']->Value) + floatval($memory_vars['innodb_additional_mem_pool']->Value) + floatval($memory_vars['net_buffer']->Value)));
  /*
  $multiply_me = ((($memory_vars['read_buffer']->Value + $memory_vars['join_buffer']->Value + $memory_vars['sort_buffer']->Value + $memory_vars['myisam_sort_buffer']->Value + $memory_vars['thread_stack']->Value + $memory_vars['tmp_table_size']->Value + $memory_vars['read_rnd_buffer']->Value) / 1024.0) / 1024.0);
  $first_part = ((($memory_vars['key_buffer']->Value + $memory_vars['innodb_buffer_pool']->Value + $memory_vars['innodb_log_buffer']->Value + $memory_vars['innodb_additional_mem_pool']->Value + $memory_vars['net_buffer']->Value + $memory_vars['max_connections']->Value) / 1024.0) / 1024.0);
  */
//  $min_memory_needed = ($memory_vars['max_connections']->Value * $per_thread_mem) + $global_buffer;
$thread_usage = $memory_vars['max_connections']->Value * $per_thread_mem;
$min_memory_needed = $thread_usage + $global_buffer;

/*
  "global_buffers" => (int) ($key_buffer->Value + $net_buffer->Value),
  "thread_buffers" => (int) ($read_rnd_buffer_size->Value + $sort_buffer->Value + $myisam_sort_buffer->Value + $read_buffer->Value + $join_buffer->Value),
  "min_memory_needed" => (int) ($global_buffers + ($thread_buffers * $max_connections->Value)));
*/
  echo "<tr><td><center<b>" . t('Minimum Memory Needed : ') . $min_memory_needed . "</b></center></td></tr>";
  echo "<tr><td>" . t('This number is a calculation of the maximum amount of memory your mySQL instance will consume on your server . It is a sum of all the caches and buffers that would benefit by not using swap or not having to goto the HD. Having a number larger than your total available amount of memory will result in swapping.') . "</td><tr>";
  echo "<tr><td><center>" . t('Min_memory_needed = key_buffer + innodb_buffer_pool + innodb_log_buffer + innodb_additional_mem_pool + net_buffer + max_connections * (read_buffer + join_buffer + sort_buffer + myisam_sort_buffer + thread_stack + tmp_table_size + read_rnd_buffer)') . "</center></td></tr>";
  echo "<tr><td><center<b>" . t('Key Buffer : ') . $memory_vars['key_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('InnoDB Buffer Pool : ') . $memory_vars['innodb_buffer_pool']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('InnoDB Log Buffer : ') . $memory_vars['innodb_log_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('InnoDB Additional Memory Pool : ') . $memory_vars['innodb_additional_mem_pool']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Net Buffer : ') . $memory_vars['net_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Max Connections : ') . $memory_vars['max_connections']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Read Buffer : ') . $memory_vars['read_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Join Buffer : ') . $memory_vars['join_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Sort Buffer : ') . $memory_vars['sort_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Myisam Sort Buffer : ') . $memory_vars['myisam_sort_buffer']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Thread Stack : ') . $memory_vars['thread_stack']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Tmp Table Size : ') . $memory_vars['tmp_table_size']->Value . "</b></center></td></tr>";
  echo "<tr><td><center<b>" . t('Read Rnd Buffer Size : ') . $memory_vars['read_rnd_buffer']->Value . "</b></center></td></tr>";
}

