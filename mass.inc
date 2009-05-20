<?php
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function BookChangeGenre ($b, $g) {
  if (!libcanedit($b)) return;
  $g = preg_replace("/'/",'',$g);
  if (!Sel("GenreId FROM libgenrelist WHERE GenreId='$g' OR GenreCode='$g'")) return;
  $s = SELECT ("GenreId FROM libgenre WHERE BookId = $b");
  while ($a1=dbf($s)) BookDelGenre($b, $a1->GenreId);
  BookAddGenre($b,$g);
}

function libMass() {
  Global $form_skip, $ProcLink, $user, $debug;
  $u = $user->uid;
  if (!user_access('выкачивать помногу')) return;
  switch (arg(1)) {
    case 'LA': return 'Load: '.LA();
      break;
    case 'getseqid':
    if ($seq = str_replace('"','"',trim($_POST[SeqName])))
      print Sel ("SeqId FROM libseqname WHERE SeqName = \"$seq\""); 
    exit;
    case 'download':
      if ($_GET['downloadall'])
        if (!$user->uid || !Sel("uid FROM libdonations WHERE uid = $user->uid"))
          if (LA() > 2) 
            return "Извините, сервер сейчас перегружен. Зайдите через полчасика.";
      $books = $dtp = $_GET['dtp'];
      foreach ($_GET as $b=>$k) {
        if (!($k == 'on' && is_numeric($b))) continue;
        if ($_GET['readedall']) {
          db_query("INSERT INTO libreaded (UserId, BookId) VALUES ($u, $b) ON DUPLICATE KEY UPDATE BookId = BookId");
        }   
        if ($_GET['downloadall']) {
          if (!$a1 = S("* FROM libbook WHERE BookId = '$b'")) continue;
          if ($a1->FileType == 'fb2') {
            if ($dtp == 'pdf') {
              $ii++;
              $pdfform .= "<input type=text size=30 name=book$ii value=http://lib.rus.ec/b/$b/fb2><br>\n";
            } else {  
              $zip = MakeZipFile($b, $dtp);
              if (file_exists($zip)) $f = $zip;    
            }  
          } else {
             $f = Sel("FileName FROM libfilename WHERE BookId = $b");
             if ($f) $f = "b.usr/$f";
          }
          if ($f && file_exists($f) && strlen($books) < 9990) {
            $filelist .= " \"$f\"";
            $books .= '.'.$b;
          }  
        }
      }      
      if ($dtp == 'pdf') 
        return  "<h2>Для конвертации выбранных книг в pdf придётся посетить другой сайт</h2>
        <form id=uploadform enctype='multipart/form-data' action=http://fb2pdf.com/batch_convert.php 
        method=POST>$pdfform<input type=submit value='отправить на конвертацию'></form>";
      if ($filelist) {
        $id = Sel ("ID FROM libmass WHERE books = '$books'");
        print "<br><br>Готовится архив. Подождите.<br><br>";
        if (!$id) {
          Insert(libmass, 'books, N', "'$books', 1");
          $id = Sel("ID FROM libmass WHERE books = '$books'");
          if (!$id) exit;
        } else {
          Update(libmass, 'N=N+1', "ID=$id");  
        }  
        $filename = "b.tmp/$id.zip";
        if (!file_exists($filename)) print system ("zip -j -0 $filename $filelist"); 
        print "<br><br><a href='/$filename'>Подготовленный архив из $iii книг</a>";
        print "<script LANGUAGE=JavaScript>window.location='/$filename'</script>";
      } 
      if ($_GET['readedall']) lgo('polka');
      exit;
    case 'setseq':
      set_title("Присвоение сериала $_GET[seqname]");
      $SeqId = 1*$_GET[seqid];
      if (!$SeqId) $SeqId = AddSeqId($_GET[seqname]);
      if (!$SeqId) return "Вернитесь назад и введите имя сериала. Не могу без него.";
      foreach ($_GET as $b=>$k) {
        if ($b > 0 && $k > 0 && libcanedit($b)) {
          BookDeleteSeq($b);
          BookAddSeq($b, $SeqId, $k);
          $r .= "Сериал присвоен: ".bl($b)."<br>";
        }
      } 
    return $r;  

    case 'undel': 
      set_title("Восстановление книг");
      $r = "<h3>Восстановление книг автора: ".avl(arg(2))."</h3>";
      foreach ($_GET as $b=>$k) {
        if ($b > 0 && $k == 'on' && libcanedit($b)) {
          UnDeleteBook($b);
          $r .= "Восстановлена книга ".bl($b)."<br>";
        }
      } 
    return $r;  

    case 'edit':
      foreach ($_GET as $b=>$k) {
        if (!($k == 'on' && is_numeric($b))) continue;
        if ($_GET['changegenre']) BookChangeGenre ($b, $_GET['genre']);
        if ($_GET['deleteallnah']) DeleteBook($b);
        if ($_GET['join']) {
          if ($b2) lgo ("b/$b/join/$b2");
          else $b2 = $b;
        }
        if ($_GET['setseq']) {
          if (!$III++) {
            set_title('Задание сериала для группы книг');
            $r .= "<form action=/mass/setseq>";
         }  
         if (preg_match('/\((.+)\s*-\s*(\d+)\)/', Sel("Title FROM libbook WHERE BookId = $b"), $m)) {
           if (!$seqname) $seqname = trim($m[1]);
         } else $m[2] = Sel("SeqNumb FROM libbook WHERE BookId = $b");  
         $bks .= "<input size=1 name=$b value='$m[2]'> ".bl($b)."<br>";
       }
       $iii++;
     } 
   if ($III) {
     $seqname = str_replace('"',"'",$seqname);
     $r .= '<script>
function confirmsetseq() {
  seq = $("input[@name=seqname]").val();
  if (!seq) return 0;
  seqid = "";
$.ajax({ url: "/mass/getseqid", async: false,type: "POST", data: "SeqName="+encodeURI(seq),
  success: function(r){$("input[@name=seqid]").val(r); seqid = r;}});
  if (seqid > 0) return 1;
  return confirm ("Сериал "+seq+" отсутствует. Создать?");
}</script>'. "Имя сериала: <input name=seqname value=\"$seqname\" ><br><br>$bks<br><input type=hidden name=seqid><input $ButtonStyle type=submit onClick='return confirmsetseq()'></form>";
return $r;
   } else {
     if ($a = arg(2)) lgo ("a/$a/edit");  
   }  
  break;  
    default: 
  return "Задачу не понял. Ничего делать не буду. Извините.";
  }
} 
