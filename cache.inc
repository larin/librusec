<?php

// APC кэш. 
// + работает быстрее любого другого способа 
// - работает внутри пыхсервера, не доступен с других серверов, 
// -+ обнуляется при перезагрузке apache

// взять из APC. 
// ID образуется автоматически из имени вызывающей функции + $id опционально
// и сохраняется для следующего вызова setapc
// нормальный сценарий: if ($r = getapc(func_get_args())) return $r; else return setpac(AnySlowFunction());
function getapc($id='') {
  Global $apc_cid;
  $f = debug_backtrace();
  $f = $f[1]['function'];
  if (!is_scalar($id)) $id = md5(serialize($id));
  $apc_cid[$f] = $f.$id;
  return apc_fetch($apc_cid[$f]);
}

// положить $val в APC на $tm секунд (0 - хранить вечно, до смерти apache). 
// ID сохраняется с предыдущего вызова getapc, который всегда идёт перед setapc.
// возвращает $val для типичного окончания одной строкой return setapc(...); вместо трёх $r = ...; setapc($r); return $r;
function setapc($val, $tm = 583) {
  Global $apc_cid;
  $f = debug_backtrace();
  $f = $f[1]['function'];
  apc_store($apc_cid[$f], $val, $tm);
  return $val;
}

