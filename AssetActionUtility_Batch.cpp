// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActionUtility_Batch.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Engine/Texture.h"

#pragma region RenameSelectedAssets

void UAssetActionUtility_Batch::RenameSelectedAssets(FString SearchPattern, FString ReplacePattern, ESearchCase::Type SearchCase)
{
	//	Check if something needs to be done
	if (SearchPattern.IsEmpty() || SearchPattern.Equals(ReplacePattern, SearchCase))
	{
		return;
	}

	//	Get the selected objects
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	//	Check each asset if it needs to be renamed
	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			FString AssetName = SelectedObject->GetName();
			if (AssetName.Contains(SearchPattern, SearchCase))
			{
				FString NewName = AssetName.Replace(*SearchPattern, *ReplacePattern, SearchCase);
				UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
				++Counter;
			}

		}
	}

	GiveFeedback(TEXT("Renamed"), Counter);
}

#pragma endregion

#pragma region CheckPowerOfTwo
void UAssetActionUtility_Batch::CheckPowerOfTwo()
{
	// Get all selected assets
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			UTexture* Texture = dynamic_cast<UTexture*>(SelectedObject);
			if (ensure(Texture))
			{
				// since it returns a float, need to do static cast to return int32
				int32 Width = static_cast<int32>(Texture->GetSurfaceWidth());
				int32 Height = static_cast<int32>(Texture->GetSurfaceHeight());

				if (!IsPowerOfTwo(Width) || !IsPowerOfTwo(Height))
				{
					PrintToScreen(SelectedObject->GetPathName() + " is not a power of 2 texture", FColor::Red);
				}
				else
				{
					++Counter;
				}
			}
			else 
			{
				PrintToScreen(SelectedObject->GetPathName() + " is not a texture", FColor::Red);
			}
		}
	}

	GiveFeedback("Power of two", Counter);
}



bool UAssetActionUtility_Batch::IsPowerOfTwo(int32 NumberToCheck)
{
	if (NumberToCheck == 0)
	{
		return false;
	}
	return (NumberToCheck & (NumberToCheck - 1)) == 0; // this will return power of two
	
}

#pragma endregion

#pragma region AddPrefixes

void UAssetActionUtility_Batch::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			const FString* Prefix = PrefixMap.Find(SelectedObject->GetClass()); //Find the class and store in String Prefix
			if (ensure(Prefix) && !Prefix->Equals("")) // If found
			{
				FString OldName = SelectedObject->GetName(); // Store the old name from the selected object
				if (!OldName.StartsWith(*Prefix)) // if the old name doesnt start with
				{
					FString NewName = *Prefix + OldName;
					UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
					++Counter;
				}
				else
				{
					PrintToScreen("Couldn't find prefix for class" + SelectedObject->GetClass()->GetName(), FColor::Red);
				}
			}
		}

		GiveFeedback("Added prefix to", Counter);
	}

}



#pragma endregion



#pragma region ProjectOrganiser

void UAssetActionUtility_Batch::CleanUpFolder(FString ParentFolder)
{
	// Check if parent folder is in content folder#
	if (!ParentFolder.StartsWith(TEXT("/Game")))
	{
		ParentFolder = FPaths::Combine(TEXT("/Game"), ParentFolder);
	}

	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			FString NewPath = FPaths::Combine(ParentFolder, SelectedObject->GetClass(), SelectedObject->GetName());
			if (UEditorAssetLibrary::RenameLoadedAsset(SelectedObject, NewPath))
			{
				++Counter;
			}
			else
			{
				PrintToScreen("Couldn't move" + SelectedObject->GetPathName(), FColor::Red);
			}
		}
	}
	GiveFeedback("Moved", Counter);
}


#pragma endregion



#pragma region DuplicateAssets

void UAssetActionUtility_Batch::DuplicateAsset(uint32 NumberOfDuplicates, bool bSave)
{
	TArray<FAssetData> AssetDataArray = UEditorUtilityLibrary::GetSelectedAssetData();

	uint32 Counter = 0;

	for (FAssetData AssetData : AssetDataArray)
	{
		for (uint32 i = 0; i < NumberOfDuplicates; ++i)
		{
			FString NewFileName = AssetData.AssetName.ToString().AppendChar('_').Append(FString::FromInt(i));
			FString NewPath = FPaths::Combine(AssetData.PackagePath.ToString(), NewFileName);
			if (ensure(UEditorAssetLibrary::DuplicateAsset(AssetData.ObjectPath.ToString(), NewPath)))
			{
				++Counter;
				if (bSave)
				{
					UEditorAssetLibrary::SaveAsset(NewPath, false);
				}
			}
		}
	}
	GiveFeedback("Duplicated", Counter);
}

#pragma endregion

#pragma region Helper

void UAssetActionUtility_Batch::PrintToScreen(FString Message, FColor Color)
{
	if (ensure(GEngine))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, Message);
	}
}

void UAssetActionUtility_Batch::GiveFeedback(FString Method, uint32 Counter)
{
	FString Message = FString("No maching files found.");
	FColor Color = Counter > 0 ? FColor::Green : FColor::Red;
	if (Counter > 0)
	{
		Message = Method.AppendChar(' ');
		Message.AppendInt(Counter);
		Message.Append(Counter == 1 ? TEXT(" file") : TEXT(" files"));
	}

	PrintToScreen(Message, Color);
}





#pragma endregion