object Controller: TController
  OldCreateOrder = False
  Height = 384
  Width = 568
  object UniQuery2: TUniQuery
    Connection = uniconn
    Left = 424
    Top = 176
  end
  object UniQuery4: TUniQuery
    Connection = uniconn
    Left = 288
    Top = 224
  end
  object UniQuery3: TUniQuery
    Connection = uniconn
    Left = 312
    Top = 136
  end
  object UniDataSource1: TUniDataSource
    DataSet = UniQuery1
    Left = 400
    Top = 240
  end
  object UniQuery1: TUniQuery
    Connection = uniconn
    Left = 352
    Top = 176
  end
  object uniconn: TUniConnection
    ProviderName = 'MySQL'
    Database = 'msgsrv_db'
    LoginPrompt = False
    Left = 400
    Top = 120
  end
  object mysql_provider: TMySQLUniProvider
    Left = 392
    Top = 56
  end
  object UniQuery5: TUniQuery
    Connection = uniconn
    Left = 312
    Top = 304
  end
end
