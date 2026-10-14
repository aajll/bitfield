foreach t : ['bitfield', 'status']
  exe = executable(
    'test_' + t,
    'test_' + t + '.c',
    dependencies: [bitfield_dep],
    install: false,
  )
  test(t + ' tests', exe)
endforeach
