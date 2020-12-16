object DataModule1: TDataModule1
  OldCreateOrder = False
  OnCreate = DataModuleCreate
  Height = 575
  Width = 738
  object SQLiteUniProvider1: TSQLiteUniProvider
    Left = 664
    Top = 32
  end
  object UniConnection1: TUniConnection
    Left = 664
    Top = 88
  end
  object UniScript1: TUniScript
    Connection = UniConnection1
    Left = 664
    Top = 144
  end
  object UniQuery1: TUniQuery
    Connection = UniConnection1
    Left = 664
    Top = 200
  end
end
